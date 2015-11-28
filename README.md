## SuperSmall Serializable psuedo-DataBase

####Installation
`make install`

####Usage
See testdriver.c for example database usage.

To test with the provided driver after compiling, run `sdb`

###TODO:
* Review API function paramaters
* Database does not persist between connections, child contains copied db struct, should talk to parent instead. 
* Persist data, save to file
