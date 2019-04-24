//
// Created by Kevin Miyata on 2019-04-24.
//
#ifndef ASSIGN7_MAIN_H
#define ASSIGN7_MAIN_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <sstream>

using namespace std;

// Global constants
const int HOW_OFTEN = 6;
const int MAX_BLOCK_SIZE = 12;
const int MAX_PRINT_SIZE = 240;
const int MAX_TABLE_SIZE = 4096;
const int BLOCK_SIZE = 512;

struct FileEntry {
    string name;
    long size = 0;
    vector<long> blocks;
};

// File Allocation Table
int fileAccessTable[MAX_TABLE_SIZE] = {};

// Vector to hold file info
vector<FileEntry> files;

void copyEntry(const string &, const string &);

void deleteEntry(const string &);

void newEntry(const string &name, long size);

void modifyEntry(const string &, long);

void renameEntry(const string &, const string &);

long searchEntry(const string &);

void printTable();

void copyEntry(const string &originalName, const string &copyName) {
    // Find the file in the table
    long index = searchEntry(originalName);
    long duplicateIndex = searchEntry(copyName);

    // If the original file cannot be found, exit
    if (index < 0) {
        cerr << "ERROR 404: File \"" << originalName << "\" could not be located!" << endl;
        return;
    }

    // If the new file name already exists, exit
    if (duplicateIndex > 0) {
        cerr << "ERROR 409: File named \"" << copyName << "\" already exists!" << endl;
        return;
    }

    // Create a new table entry with the old information
    newEntry(copyName, files.at(index).size);

    cerr << "File \"" << originalName << "\" has been copied into new file \"" << copyName << "\"!" << endl;
}

void deleteEntry(const string &name) {
    // Find the file's location in the table
    long location = searchEntry(name);

    // If the location of the file cannot be found, exit
    if (location == -1) {
        cerr << "ERROR 404: File \"" << name << "\" could not be located!" << endl;
        return;
    } else {
        // Clear table entry memory blocks
        for (const auto &item : files.at(location).blocks) {
            fileAccessTable[item] = 0;
        }

        // Remove file from table listing and print result
        files.erase(files.begin() + location);
        cerr << "File \"" << name << "\" has been deleted!" << endl;
    }
}


void newEntry(const string &name, long size) {
    long blockCount;
    FileEntry tempFile;

    // Check if the file name has already been taken, then exit
    if (searchEntry(name) != -1) {
        cerr << "ERROR 409: File name \"" << name << "\" is already in use!" << endl;
        return;
    }

    // Create temporary file entry
    tempFile.name = name;
    tempFile.size = size;

    // Calculate the required amount of blocks. If there is a remainder, add an extra 1.
    if (size % BLOCK_SIZE > 0) {
        blockCount = size / BLOCK_SIZE;
        blockCount = blockCount + 1;
    } else {
        blockCount = size / BLOCK_SIZE;
    }

    int count = 1;

    // Loop through the blocks that are needed
    while (count <= blockCount) {
        for (int i = 0; i < MAX_TABLE_SIZE; i++) {

            // Set the last block to -1
            if (fileAccessTable[i] == 0 && count == blockCount) {
                fileAccessTable[i] = -1;
                tempFile.blocks.push_back(i);
                count++;
                break;
            } else if (fileAccessTable[i] == 0) {
                // Mark the data blocks as in-use
                fileAccessTable[i] = 1;
                tempFile.blocks.push_back(i);
                count++;
                break;
            }
        }
    }

    // Push new file to the file access table listing
    files.push_back(tempFile);

    cerr << "A new file named \"" << name << "\" has been created!" << endl;
}


void modifyEntry(const string &name, long size) {
    // Find the file's location in the file access table.
    long location = searchEntry(name);

    // Check for the existence of the file in question
    if (location == -1) {
        cout << "ERROR 404: File \"" << name << "\" could not be located!" << endl;

        // If none is found, then exit.
        return;
    } else {
        for (auto &blocky: files.at(location).blocks) {
            // Clear data to 0
            fileAccessTable[blocky] = 0;
        }

        // Remove the file from the data table
        files.erase(files.begin() + location);

        // Create a new file with the same name
        newEntry(name, size);

        cerr << "File \"" << name << "\" has been modified with a new size of " << size << "!" << endl;
    }
}

void renameEntry(const string &oldName, const string &newName) {
    // Search for the original file name
    long location = searchEntry(oldName);

    // Check if the original file name is in the table, exit if it could not be located or already exists
    if (location == -1) {
        cerr << "ERROR 404: File named \"" << oldName << "\" could not be located!" << endl;
        return;
    } else if (searchEntry(newName) >= 0) {
        cerr << "ERROR 409: File named \"" << oldName << "\" already exists!" << endl;
        return;
    } else {
        // Modify the entry's name in the table
        files.at(location).name = newName;
        cerr << "Old file \"" << oldName << "\" has been renamed to \"" << newName << "\"!" << endl;
    }
}

long searchEntry(const string &entryName) {
    int position = 0;

    for (const auto &i: files) {
        // Check if the current entry's name matches the scanned entry
        if (i.name == entryName) {
            return position;
        }
        position++;
    }

    // Return -1 if no such file name was found in the table
    return -1;
}

void printTable() {
    long totalSize = 0;

    for (const auto &item: files) {
        cerr << "File name: " << setw(20) << left << item.name << "\t" << "File size: " << item.size << endl;
        totalSize += item.size;

        cerr << "Clusters in use: ";
        long indent = 1;

        for (const auto &blocky: item.blocks) {
            cerr << setw(6) << right << blocky;
            if (indent % MAX_BLOCK_SIZE == 0) {
                cerr << endl;
            }
            indent++;
        }

        if (item.blocks.empty()) {
            cerr << "None!";
        }

        cerr << endl << endl;
    }

    cerr << "Files: " << files.size() << "\t" << "File size:" << totalSize << endl << endl;
    int indent = 0;

    for (unsigned long i = 0; i < MAX_PRINT_SIZE; i++) {
        if (indent % MAX_BLOCK_SIZE == 0) {
            cerr << "#" << setw(3) << setfill('0') << i << " - " << setw(3) << setfill('0') << i + 11 << ": " << "\t"
                 << setfill(' ');
        }

        cerr << right << setw(2) << fileAccessTable[i] << "\t";
        indent++;

        if (indent % MAX_BLOCK_SIZE == 0) {
            cerr << endl;
        }
    }

    cerr << endl;
}

#endif //ASSIGN7_MAIN_H
