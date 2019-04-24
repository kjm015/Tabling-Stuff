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

int searchEntry(const string &);

void printTable();


int main(int argc, char *argv[]) {
    // Print counter
    int printCounter = 0;

    // Open input file
    ifstream file;
    file.open("data7.txt");
    string line;

    // Create parent and this directory
    newEntry(".", 512);
    newEntry("..", 0);

    // Print table contents
    printTable();

    // Cycle through the entire input file
    while (!file.eof()) {
        char type;
        string name;
        string name2;
        int size;

        // Read input line and tokenize it
        getline(file, line);
        istringstream inLine(line);

        // Determine transaction type from input line
        if (line.at(0) == 'C') {
            inLine >> type >> name >> name2;
            copyEntry(name, name2);
        } else if (line.at(0) == 'D') {
            inLine >> type >> name;
            deleteEntry(name);
        } else if (line.at(0) == 'N') {
            inLine >> type >> name >> size;
            newEntry(name, size);
        } else if (line.at(0) == 'M') {
            inLine >> type >> name >> size;
            modifyEntry(name, size);
        } else if (line.at(0) == 'R') {
            inLine >> type >> name >> name2;
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

void copyEntry(const string &name, const string &name2) {
    // Find the file in the table
    int index = searchEntry(name);
    int noDup = searchEntry(name2);

    // If the original file cannot be found, exit
    if (index < 0) {
        // TODO: Modify output statements
        cerr << "ERROR COPY: File " << name << " was not found." << endl;
        return;
    }

    // If the new file name already exists, exit
    if (noDup > 0) {
        // TODO: Modify output statements
        cerr << "ERROR COPY: File " << name2 << " was already in use." << endl;
        return;
    }

    // Create a new table entry with the old information
    newEntry(name2, files.at(index).size);

    // TODO: Modify output statements
    cerr << "COPY: File " << name << " has been copied to " << name2 << endl;

}

void deleteEntry(const string &name) {
    // Find the file's location in the table
    int location = searchEntry(name);

    // If the location of the file cannot be found, exit
    if (location == -1) {
        // TODO: Modify output statements
        cerr << "ERROR DELETE: File " << name << " was not found." << endl;
        return;
    } else {
        // Delete the Entry's blocks by assigning them to 0
        for (unsigned int i = 0; i < files.at(location).blocks.size(); i++) {
            FAT[files.at(location).blocks.at(i)] = 0;
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
        // TODO: Modify output statements
        cerr << "ERROR NEW: File " << name << " is already in use." << endl;
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

    // TODO: Modify output statements
    cerr << "NEW: File " << name << " has been created." << endl;
}


void modifyEntry(const string &name, int size) {
    // Find the file's location in the file access table.
    int location = searchEntry(name);

    // Check for the existence of the file in question
    if (location == -1) {
        // TODO: Modify output statements
        cout << "ERROR MODIFY: File " << name << " was not found." << endl;

        // If none is found, then exit.
        return;
    } else {
        // Set data to 0
        for (auto &blocky: files.at(location).blocks) {
            FAT[blocky] = 0;
        }

        // Remove the file from the data table
        files.erase(files.begin() + location);

        // Create a new file with the same name
        newEntry(name, size);

        // TODO: Modify output statements
        cerr << "MODIFY: File " << name << " has been modified." << endl;
    }
}

void renameEntry(const string &name, const string &name2) {
    // Search for the original file name
    unsigned long findName = searchEntry(name);

    // Check if the original file name is in the table
    if (findName == -1) {
        // TODO: Modify output statements
        cerr << "ERROR RENAME: File " << name << " was not found." << endl;

        // Exit if the file could not be located
        return;
    } else if (searchEntry(name2) >= 0) {
        // TODO: Modify output statements
        cerr << "ERROR COPY: File " << name << " is already in use." << endl;

        // Exit if the new file name has already been taken
        return;
    } else {
        // Modify the entry's name in the table
        files.at(findName).name = name2;

        // TODO: Modify output statements
        cerr << "RENAME: File " << name << " has been Renamed." << endl;
    }
}

int searchEntry(const string &name) {
    int position = 0;

    for (const auto &i: files) {
        // Check if the current entry's name matches the scanned entry
        if (i.name == name) {
            return position;
        }
        position++;
    }

    // Return -1 if no such file name was found in the table
    return -1;
}

void printTable() {
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

        // TODO: Modify output statements
        cerr << "File Name: " << file.name << "\t File Size: " << file.size << "\t Blocks Used: ";

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

    // TODO: Modify output statements
    cerr << "Total number of files: " << files.size() - 1 << "\t Total Size: " << total << endl << endl;
}
