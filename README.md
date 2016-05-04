## SuperSmall Serializable psuedo-DataBase

####Installation
`make install`

####Usage
See testdriver.c for example database usage.

To test with the provided driver after compiling, run `sdb`

Example database operations:

* List table with id 100
  `lt t 100`
* List all tables
  `la`
* Add entry to table with id 100 and contents hello
  `ae 100 hello`
* Delete entry 100 from table 101
  `de 101 100`
* Add table with name mytable
  `at mytable`

###TODO:
* Review API function paramaters
* Add lockfile to in/externalizers
* Fix buffer overflows
