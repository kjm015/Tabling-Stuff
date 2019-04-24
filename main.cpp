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

// Entry block for each file in the table
struct Entry {
    string name;
    int size = 0;
    vector<unsigned long> blocks;
};

// File Allocation Table
int FAT[MAX_TABLE_SIZE] = {};

// Vector to hold file info
vector<Entry> files;

void copyEntry(const string &, const string &);

void deleteEntry(const string &);

void newEntry(const string &, int size);

void modifyEntry(const string &, int size);

void renameEntry(const string &, const string &);

long searchEntry(const string &);

void printTable();

int main(int argc, char *argv[]) {
    // Print counter
    int printCounter = 0;

    // Open input file
    ifstream file;
    file.open("data7.txt");
    string inputLine;

    // Create parent and this directory
    newEntry(".", 512);
    newEntry("..", 0);

    // Print table contents
    printTable();

    // Cycle through the entire input file
    while (!file.eof()) {
        vector<string> tokens;

        char type;
        string name;
        string name2;
        int size;

        // Read input line and tokenize it
        getline(file, inputLine);

        istringstream streamer(inputLine);

        // Determine transaction type from input line
        if (inputLine.at(0) == 'C') {
            streamer >> type >> name >> name2;
            copyEntry(name, name2);
        } else if (inputLine.at(0) == 'D') {
            streamer >> type >> name;
            deleteEntry(name);
        } else if (inputLine.at(0) == 'N') {
            streamer >> type >> name >> size;
            newEntry(name, size);
        } else if (inputLine.at(0) == 'M') {
            streamer >> type >> name >> size;
            modifyEntry(name, size);
        } else if (inputLine.at(0) == 'R') {
            streamer >> type >> name >> name2;
            renameEntry(name, name2);
        } else {
            break;
        }

        printCounter++;

        if (printCounter % HOW_OFTEN == 0) {
            printTable();
        }
    }

    // Close the file
    file.close();

    // Print final table state and exit
    printTable();
    return 0;
}

void copyEntry(const string &originalName, const string &copyName) {
    // Find the file in the table
    long index = searchEntry(originalName);
    long duplicateIndex = searchEntry(copyName);

    // If the original file cannot be found, exit
    if (index < 0) {
        cerr << "ERROR: File \"" << originalName << "\" could not be located!" << endl;
        return;
    }

    // If the new file name already exists, exit
    if (duplicateIndex > 0) {
        cerr << "ERROR: File named \"" << copyName << "\" already exists!" << endl;
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
        // TODO: Modify output statements
        cerr << "ERROR DELETE: File " << name << " was not found." << endl;
        return;
    } else {
        // Delete the Entry's blocks by assigning them to 0
        for (auto &item : files.at(location).blocks) {
            FAT[item] = 0;
        }

        // Remove file from table listing
        files.erase(files.begin() + location);

        // TODO: Modify output statements
        cerr << "DELETE: File " << name << " has been deleted." << endl;
    }
}


void newEntry(const string &name, int size) {
    int blockCount;
    Entry tempFile;

    // Check if the file name has already been taken, then exit
    if (searchEntry(name) != -1) {
        cerr << "ERROR: File name \"" << name << "\" is already in use!" << endl;
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
            if (FAT[i] == 0 && count == blockCount) {
                FAT[i] = -1;
                tempFile.blocks.push_back(i);
                count++;
                break;
            } else if (FAT[i] == 0) {
                // Mark the data blocks as in-use
                FAT[i] = 1;
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


void modifyEntry(const string &name, int size) {
    // Find the file's location in the file access table.
    long location = searchEntry(name);

    // Check for the existence of the file in question
    if (location == -1) {
        cout << "ERROR: File \"" << name << "\" could not be located!" << endl;

        // If none is found, then exit.
        return;
    } else {
        for (auto &blocky: files.at(location).blocks) {
            // Clear data to 0
            FAT[blocky] = 0;
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

void print() {
    int total = 0;

    // TODO: Modify output statements
    cerr << endl << "Blocks occupied: " << endl;

    int i = 0;

    // Print the entries' data blocks
    while (i < MAX_PRINT_SIZE) {
        cerr << i << " - " << i + MAX_BLOCK_SIZE << ": \t";

        // Print the blocks' statuses
        for (int j = 0; j <= MAX_BLOCK_SIZE; j++) {
            cerr << FAT[j] << "\t";
        }

        i = i + MAX_BLOCK_SIZE;
        cerr << endl;
    }

    cerr << endl;

    // Print the memory blocks for each file
    for (auto &file: files) {

        cerr << "File name: " << file.name << "\t File size: " << file.size << "\t Blocks used: ";

        // If the file takes no space, then print a default value
        if (file.blocks.empty()) {
            cerr << "None!";
        }

        total = total + file.size;

        for (auto &blocky: file.blocks) {
            cerr << blocky << " ";
        }

        cerr << endl;
    }

    cerr << "Total number of files: " << files.size() - 1 << "\t Total file size: " << total << endl << endl;
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
            if (indent % 12 == 0) {
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

    for (unsigned long i = 0; i < 240; i++) {
        if (indent % 12 == 0) {
            cerr << "#" << setw(3) << setfill('0') << i << " - " << setw(3) << setfill('0') << i + 11 << ": " << "\t"
                 << setfill(' ');
        }

        cerr << right << setw(2) << FAT[i] << "\t";
        indent++;

        if (indent % 12 == 0) {
            cerr << endl;
        }
    }

    cerr << endl;
}
