#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <sstream>
#include <iomanip>
#include <sstream>
#include <cstring>

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

void createBinary(string &fileName);
string getOutputFile(string &fileName);
void collectData(fstream &inFile, fstream &outFile, BookRec &records, string &fileName, map<unsigned int, bool> &legal);
void legalCheck(int &count, BookRec &records, map<unsigned int, bool> &legal, unsigned int &greatest_isbn);
void printLine(BookRec &records);
void addToBinFile(BookRec &records, string &fileName, fstream &inFile, fstream &outFile, map<unsigned int, bool> &legal);

int main(int argLength, const char* args[2]) {

        cout << endl;

        //Only allows two args
        if (argLength != 2) {
                cout << "Invalid args." << endl;
                return 0;
}
        string fileName = args[1];
        ifstream file(fileName);

        //Exit program if file does not exist
        if (!file) {
                cout << "File not found." << endl;
                return 0;
}
        createBinary(fileName); //Start program
}

void createBinary(string &fileName) {

        fstream inFile(fileName.c_str(), ios::in); //File for reading in
        fstream outFile(getOutputFile(fileName).c_str(), ios::out | ios::binary); //File for writing into binary file

        BookRec records;

        map<unsigned int, bool> legal; //map to keep track of record legality

        collectData(inFile, outFile, records, fileName, legal);

        cout << endl;

}

void collectData(fstream &inFile, fstream &outFile, BookRec &records, string &fileName, map<unsigned int, bool> &legal) {

        binString line;
        unsigned int prev_isbn = 0; //Variable that checks if the next ISBN is out of place
        int lineNum = 1; //Variable for telling user which line they are on
        ifstream reader(fileName); //Reads in the data

        //Reads through the whole data file to check which records are legal and illegal
        while (reader.getline(line, 50, '|')) {
                long isbn = stol(line);   //stol function gotten from: https://www.geeksforgeeks.org/cpp-program-for-string-to-long-conversion/

                if (isbn > -1)
                        records.isbn = isbn;

                else {
                        cout << "Illegal ISBN found on line " << lineNum << ". Record ignored." << endl << endl;
                        legal[isbn] = false;
}
 
                reader.getline(line, 50, '|');
                strcpy(records.name, line);

                reader.getline(line,50, '|');
                strcpy(records.author, line);

                reader.getline(line,50, '|');
                records.onhand = stoi(line); 

                reader.getline(line,50, '|');
                records.price = stod(line); //stod function gotten from: https://www.geeksforgeeks.org/cpp-program-for-string-to-double-conversion

                reader.getline(line,50, '\n');
                strcpy(records.type, line);

                legalCheck(lineNum, records, legal, prev_isbn);

                if (legal[records.isbn])
                        prev_isbn = records.isbn;

                lineNum++;
}
                addToBinFile(records, fileName, inFile, outFile, legal); //Writes into binary file
                fstream binIn (getOutputFile(fileName).c_str(), ios::in | ios::binary); //Reads from new binary file


        //Prints the binary file in readable form
       while (binIn.read((char *) &records, sizeof(records))) {           
                printLine(records);
}
}

void addToBinFile(BookRec &records, string &fileName, fstream &inFile, fstream &outFile, map<unsigned int, bool> &legal) {

        char temp = '|';

        //Reads through each individual field in the data file and writes the legal records into the binary file
        while (inFile >> records.isbn) {
                inFile >> temp;
                inFile.getline(records.name, 50, '|');
                inFile.getline(records.author, 50, '|');
                inFile >> records.onhand >> temp >> records.price;
                inFile >> temp;
                inFile.getline(records.type, 50, '\n');

                if (legal[records.isbn])
                        outFile.write((char*) &records, sizeof(BookRec));
}
                inFile.close();
                outFile.close();
}

string getOutputFile(string &fileName) {

        //Puts a ".out" extension on the original file name for its binary version

        int found = fileName.find('.');

        if (found == -1)
                return fileName + ".out";

        return fileName.substr(0, found) + ".out";
}

void legalCheck(int &count, BookRec &records, map<unsigned int, bool> &legal, unsigned int &prev_isbn) {

        legal[records.isbn] = true;

        //If the isbn is 0, ignore it and inform the user
        if (records.isbn == 0) {
                cerr << "Illegal ISBN number 0 at line " << count << ". The following record is ignored:" << endl;
                printLine(records);
                legal[records.isbn] = false;
                cout << endl;
}
        //If the isbn is legal but out of place, inform the user
        if (records.isbn < prev_isbn && records.isbn > 0) {
                cerr << "ISBN number out of order at line " << count << ". Here's the record:" << endl;
                printLine(records);
                cout << endl;
}
        //If the price is negative, ignore it and inform the user
        if (records.price < 0) {
                cerr << "Negative price at line " << count << ". The following record is ignored:" << endl;
                printLine(records);
                legal[records.isbn] = false;
                cout << endl;
}
        //If the onhand is negative, ignore it and inform the user
        if (records.onhand < 0) {
                cerr << "Negative onhand at line " << count << ". The following record is ignored:" << endl;
                printLine(records);
                legal[records.isbn] = false;
                cout << endl;
}
}

void printLine(BookRec &records) {

        //Prints records to screen
        printf("%010u%25s%25s%3d%7.2f%10s\n", records.isbn, records.name, records.author, records.onhand, records.price, records.type);
}
