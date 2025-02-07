The program aims to create a binary file of book records of an existing data file, which can then be updated by a transaction file.

The Files:

-       library.dat: file that contains the original book records in readable form.  Its data gets read in and translated into binary in a new file library.out
-       library.out: file that contains all legal book records from library.dat in binary form
-       copy.out: copy of library.out that gets modified by the update.cpp program via the transact.out file
-       transact.out: Lists all of the desired modifications for the library to be written into the update.out file
-       update.out: the updated library after finishing the transaction

Outline of Algorithm:

-       Check if data file exists
-       Read through data file and check the legality of each record
-       Create a new binary file and write in all legal records into it
-       Create a copy of the binary file
-       Modify the copy based on the transactions read in from transact.out
-       Print all items on record into a new binary file update.out
