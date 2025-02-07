#include <fstream>
#include <iostream>
#include <map>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

typedef char binString[25];
struct BookRec {
	unsigned int isbn;
	binString name;
	binString author;
	int onhand;
	float price;
	binString type;
};

enum TransactionType {Add, Delete, ChangeOnhand, ChangePrice};
struct TransactionRec {

	TransactionType cmd;
	BookRec b;
};

void updateBin(string &mastBin, string &transBin, string &newBin);
void transaction(map<unsigned int, long> &isbnList, map<unsigned int, bool> &onRecord, TransactionRec &trans, string &transBin, string &newLib, long &offset);
bool isDuplicate(string &newLib, TransactionRec &trans);
bool exists (string &newLib, TransactionRec &trans);
void updateOnhand(unsigned int &isbn, int &onhand, string &newLib, TransactionRec &trans, map<unsigned int, long> &isbnList);
void updatePrice(unsigned int &isbn, float &price, string &newLib, TransactionRec &trans, map<unsigned int, long> &isbnList);
void sortISBN(vector<unsigned int> &isbnSort, string &newLib);
void printNew(string &newBin, string &newLib, vector<unsigned int> &isbnSort, map<unsigned int, long> &isbnList, map<unsigned int, bool> &onRecord);
void printLine(BookRec &records);

int main(int argc, char* args[4]) {

	//If the number of args is not 4, exit program
	if (argc != 4) {
		cerr << "Invalid args." << endl;
		return 0;
	}

	string mastBin = args[1];
	ifstream file1(mastBin);
	string transBin = args[2];
	ifstream file2(transBin);

	//If either the original master file or transaction file don't exist, exit program
	if (!file1 || !file2) {
		cerr << "Master file or transaction file not found." << endl;
		return 0;
	}
	file1.close();
	file2.close();

	string newBin = args[3];

	updateBin(mastBin, transBin, newBin); //Start program
}

void updateBin(string &mastBin, string &transBin, string &newBin) {

	TransactionRec trans;

	string newLib = "copy.out"; //Name of copy for master file

	fstream oldFile(mastBin.c_str(), ios::in | ios::binary); //Reads from original master file
	fstream mastWrite(newLib.c_str(), ios::out | ios::binary); //Writes into copy

	BookRec brec;

	//Take the contents of the original master file and write them into the copy
	while (oldFile.read((char*) &brec, sizeof(BookRec))) {
		mastWrite.write((char*) &brec, sizeof(BookRec));
	}
	oldFile.close();
	mastWrite.close();

	map<unsigned int, long> isbnList; //Map keeping track of each isbn's byte offset
	map<unsigned int, bool> onRecord; //Checks if we want to keep the record (true if yes false if no)

	long offset = 0;

	fstream mastRead(newLib.c_str(), ios::in | ios::binary); //Reads from new master file

	//Sets up the byte offset map
	while (mastRead.read((char*) &brec, sizeof(BookRec))) {
		isbnList[brec.isbn] = offset;
		onRecord[brec.isbn] = true;
		offset += sizeof(BookRec);
	}
	mastRead.close();

	transaction(isbnList, onRecord, trans, transBin, newLib, offset); //Do transaction

	vector<unsigned int> isbnSort; //Store the isbn's once all records are read in

	sortISBN(isbnSort, newLib); //Sort isbn's for updated binary file

	printNew(newBin, newLib, isbnSort, isbnList, onRecord); //Creates new binary file and sorts them by isbn
}

void transaction(map<unsigned int, long> &isbnList, map<unsigned int, bool> &onRecord, TransactionRec &trans, string &transBin, string &newLib, long &offset) {

	fstream transRead(transBin.c_str(), ios::in | ios::binary); //Reader for the transaction file
	fstream mastWrite(newLib.c_str(), ios::out | ios::binary | ios::app); //Writes into copied master file
	ofstream errors("ERRORS"); //Prints errors into a text file

	int transCount = 1; //Keeps track of the transactions so the user can know which transaction they are on

	while (transRead.read((char*) &trans, sizeof(TransactionRec))) {

		switch (trans.cmd) {
		//If the isbn is a new record, write it into the copied master file and put it on the record
		case Add:
			if(!isDuplicate(newLib, trans)) {
				mastWrite.write((char*) &trans.b, sizeof(BookRec));
				isbnList[trans.b.isbn] = offset;
				onRecord[trans.b.isbn] = true;
				offset += sizeof(BookRec);
			}
			else {
				errors << "Error on transaction " << transCount << ": cannot add duplicate key " << trans.b.isbn << endl;
			}
			break;

			//If the record exists, take it off the record 
		case Delete:
			if (exists(newLib, trans)) 
				onRecord[trans.b.isbn] = false;

			else {
				errors << "Error on transaction " << transCount << ": record " << trans.b.isbn << " does not exist and cannot be deleted." << endl;
			}
			break;

			//If the record exists, add the onhand by the number in the transaction record
		case ChangeOnhand:
			if (exists(newLib, trans)) 
				updateOnhand(trans.b.isbn, trans.b.onhand, newLib, trans, isbnList);

			else {
				errors << "Error on transaction " << transCount << ": record " << trans.b.isbn << " does not exist and cannot have its onhand changed." << endl;
			}
			break;

			//If the record exists, add the price by the number in the transaction record
		case ChangePrice:
			if (exists(newLib, trans)) {
				updatePrice(trans.b.isbn, trans.b.price, newLib, trans, isbnList);
			}
			else {
				errors << "Error on transaction " << transCount << ": record " << trans.b.isbn << " does not exist and cannot have its price changed." << endl;
			}
			break;

			//Informs the user if they have an invalid TransactionType in the transaction file
		default:
			errors << "Error on transaction " << transCount << ": invalid TransactionType field." << endl;
		}
		transCount++;
	}
	transRead.close();
	mastWrite.close();               
}

bool isDuplicate(string &newLib, TransactionRec &trans) {

	return exists(newLib, trans); //If the file exists, it's a duplicate
}

bool exists (string &newLib, TransactionRec &trans) {

	fstream mastRead(newLib.c_str(), ios::in | ios::binary);

	BookRec brec;

	//Searches through the copied master file and checks for a matching isbn 
	while (mastRead.read((char*) &brec, sizeof(BookRec))) {
		if (brec.isbn == trans.b.isbn) {
			mastRead.close();
			return true;
		}
	}
	mastRead.close();
	return false;
}

void updateOnhand(unsigned int &isbn, int &onhand, string &newLib, TransactionRec &trans, map<unsigned int, long> &isbnList) {

	//Read through the original master file, find the byte offset to change the onhand, and overwrite that record with the new onhand

	fstream mastFile(newLib.c_str(), ios::in | ios::out | ios::binary);

	BookRec brec;

	mastFile.seekg(isbnList[isbn]);

	mastFile.read((char*) &brec, sizeof(BookRec));

	trans.b.onhand = brec.onhand + onhand;

	mastFile.seekp(isbnList[isbn]);

	mastFile.write((char*) &trans.b, sizeof(BookRec));

	mastFile.close();
}

void updatePrice(unsigned int &isbn, float &price, string &newLib, TransactionRec &trans, map<unsigned int, long> &isbnList) {

	//Read through the original master file, find the byte offset to change the price, and overwrite that record with the new price

	fstream mastFile(newLib.c_str(), ios::in | ios::out | ios::binary);

	BookRec brec;

	mastFile.seekg(isbnList[isbn]);

	mastFile.read((char*) &brec, sizeof(BookRec));

	trans.b.price = brec.price + price;

	mastFile.seekp(isbnList[isbn]);

	mastFile.write((char*) &trans.b, sizeof(BookRec));

	mastFile.close();
}

void sortISBN(vector<unsigned int> &isbnSort, string &newLib) {

	fstream mastRead (newLib.c_str(), ios::in | ios::binary); //Read through the master file to retrieve the isbn numbers

	BookRec brec;

	//Store the isbn numbers in the vector
	while (mastRead.read((char*) &brec, sizeof(BookRec))) 
		isbnSort.push_back(brec.isbn);

	mastRead.close();

	sort(isbnSort.begin(), isbnSort.end()); //Sort the isbn numbers in ascending order
}

void printNew(string &newBin, string &newLib, vector<unsigned int> &isbnSort, map<unsigned int, long> &isbnList, map<unsigned int, bool> &onRecord) {

	fstream libRead(newLib.c_str(), ios::in | ios::binary); //Read through the updated copy of the master file
	fstream newBinWrite(newBin.c_str(), ios::out | ios::binary); //Writes into the new binary file

	BookRec brec;

	//Reads through the copied master file and uses the byte offsets stored in isbnList to write the records in ascending isbn order
	for (int i = 0; i < isbnSort.size(); i++) {
		if (onRecord[isbnSort[i]]) {
			libRead.seekg(isbnList[isbnSort[i]]);
			libRead.read((char*) &brec, sizeof(BookRec));
			newBinWrite.write((char*) &brec, sizeof(BookRec));
		}
	}

	libRead.close();
	newBinWrite.close();

	fstream newBinRead(newBin.c_str(), ios::in | ios::binary); //Reads the new binary file

	//Prints the new binary file to screen in readable form
	while (newBinRead.read((char*) &brec, sizeof(BookRec)))
		printLine(brec);

	newBinRead.close();

	remove(newLib.c_str());
}

void printLine(BookRec &records) {

	//Prints the file while formatting it
	printf("%010u%25s%25s%3d%7.2f%10s\n", records.isbn, records.name, records.author, records.onhand, records.price, records.type);
}
