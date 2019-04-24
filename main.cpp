//
// Created by Kevin Miyata on 2019-04-23.
//
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <sstream>

#include "main.h"

using namespace std;

int main() {
    cerr << "Hello, world!" << endl << endl;

    // Open input file
    ifstream inputFile;
    inputFile.open("data7.txt");
    string inputLine;

    // Create parent and this directory
    newEntry(".", 512);
    newEntry("..", 0);

    // Print table contents
    printTable();

    // Create variables for program run
    bool shouldContinue = true;
    long printCounter = 0;

    // Cycle through the entire input file
    while (!inputFile.eof() and shouldContinue) {
        char type;
        string name;
        string name2;
        int size;

        // Read input line and tokenize it
        getline(inputFile, inputLine);
        istringstream streamer(inputLine);

        // Determine transaction type from input line
        switch (inputLine.at(0)) {
            case 'C':
                streamer >> type >> name >> name2;
                copyEntry(name, name2);
                break;

            case 'D':
                streamer >> type >> name;
                deleteEntry(name);
                break;

            case 'M':
                streamer >> type >> name >> size;
                modifyEntry(name, size);
                break;

            case 'N':
                streamer >> type >> name >> size;
                newEntry(name, size);
                break;

            case 'R':
                streamer >> type >> name >> name2;
                renameEntry(name, name2);
                break;

            default:
                shouldContinue = false;
                break;
        }

        printCounter++;

        // Print table every HOW_OFTEN number of cycles
        if (printCounter % HOW_OFTEN == 0) {
            printTable();
        }
    }

    // Close the file when finished
    inputFile.close();

    // Print final table state and exit
    printTable();
    cerr << endl << "Program run complete!" << endl;

    return 0;
}
