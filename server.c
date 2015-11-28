/* server.c
 * Simple database server to demo live-migration
 * Created with the help of Beej's network programming guide for C
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include "sdb.h"

#define PORT "8900"
#define BACKLOG 10

#define ERRSTR  "Unable to perform operation\n"
#define ENTRY_WROTE  "Wrote to entry %d\n"
#define ENTRY_DELETE  "Deleted entry\n"

#define TABLE_CREATE "Created table with id %d\n"
#define TABLE_DELETE "Deleted table\n"

#define GREETING "Connected to Super-simple serializable DataBase (SDB)\n"
#define GOODBYE "Goodbye!\n"
#define MAX_DIGITS 10

void sigchld_handler(int s) {
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/* lt 100 - list table with id
   la - list all tables
   ae 100 str - add entry to table w/id w/string
   de 101 100 - delete entry 100 from table 101
   at str - add table with name
*/

char * handle_input(DB * db, char * input) {
	char op[4], val[256], *res = NULL;
	int id;
	sscanf(input, "%2s %d %s", op, &id, val);
	if (strcmp(op, "lt") == 0) {
	// lt = list table
		Table * t = table_for_id(db, id);
		if (t)
			res = all_data_from_table(t);
	} else if (strcmp(op, "la") == 0) {
		// la = list all tables
		res = all_tables(db);
	} else if (strcmp(op, "ae") == 0) {
		// ae = add entry
		int newentry = write_entry(db, id, -1, val);
		if (newentry >= 0) {
			res = malloc(strlen(ENTRY_WROTE) + newentry + MAX_DIGITS);
			sprintf(res, ENTRY_WROTE, newentry);
		}
	} else if (strcmp(op, "de") == 0) {
		// de = delete entry
		Table * t = table_for_id(db, id);
		if (t) {
			id = atoi(val);
			remove_entry(t, id);
			res = malloc(strlen(ENTRY_DELETE) + 1);
			strcpy(res, ENTRY_DELETE);
		}
	} else if (strcmp(op, "at") == 0) {
		// at = addtable
		val[0] = '\0';
		sscanf(input, "%2s %s", op, val);
		Table * t = create_table(db, val);
		if (t) {
			res = malloc(strlen(TABLE_CREATE) + 1 + MAX_DIGITS);
			sprintf(res, TABLE_CREATE, t->id);
		}
	} else if (strcmp(op, "dt") == 0) {
		// dt = delete table
		Table * t = table_for_id(db, id);
		if (t) {
			remove_table(db, id);
			res = malloc(strlen(TABLE_DELETE) + 1);
			strcpy(res, TABLE_DELETE);
		}
	}
	if (res == NULL) {
		res = malloc(strlen(ERRSTR) + 1);
		strcpy(res, ERRSTR);
	}
	return res;
}


int main(int argc, char ** argv) {
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	// DB 
	DB * db = create_db("db1");

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		// try open socket
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
						p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		// try set socket options
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
					sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		// try bind to socket
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: bind successful, listening...\n");

	while(1) {
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
				get_in_addr((struct sockaddr *)&their_addr),
				s, sizeof s);
		printf("server: got connection from %s\n", s);
		/*int pipefd[2];
		char buf;
		pipe(pipefd);*/
		if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener
			// close read end of pipe
			//close(pipefd[0]);
			// yup buffer overflow
			char buf[1024], * parse_res = NULL; 
			int nbytes;
			send(new_fd, GREETING, strlen(GREETING)+1, 0);
			while (1) {
				// instead of dealing with pipes, we will serialize
				// and unserialize for each operation
				buf[0] = '\0';
				
				if ((nbytes = recv(new_fd, buf, sizeof buf, 0)) <= 0)
					break;

				printf("received %s", buf);

				if (strncmp(buf, "exit", 4) == 0) {
					send(new_fd, GOODBYE, strlen(GOODBYE)+1, 0);
					break;
				}
				
				if (buf[0] != '\0' && buf[0] != '\n' && buf[0] != '\r') {
					parse_res = handle_input(db, buf);
					/*write(pipefd[1], buf, strlen(buf)+1);
					close(pipefd[1]);
					*/	
					printf("sending: %s", parse_res);

					if (send(new_fd, parse_res, strlen(parse_res)+1, 0) == -1) 
						break;
				}
			}
			close(new_fd);

			if (parse_res)
				free(parse_res);

			/*write(pipefd[1], "exit", 5);
			close(pipefd[0]);
			close(pipefd[1]);*/
			printf("child is exiting\n");
			exit(0);

		}
		close(new_fd);  // parent doesn't need this
	}
	destroy_db(db);

	return 0;
}

