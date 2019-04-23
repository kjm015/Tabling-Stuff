#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cstddef>
#include <algorithm>
#include <list>
#include <iomanip>

using namespace std;

const unsigned int HOW_OFTEN = 6;
const unsigned int MAX_ENTRIES = 12;
const unsigned int BLOCK_SIZE = 512;
const unsigned int TABLE_SIZE = 4096;

const char *fileName = "data.txt";

vector<int> directory(TABLE_SIZE, 0);

void createFile(const string &);

void deleteFile(const string &);

void copyFile(const string &);

void renameFile(const string &);

void modifyFile(const string &);

void print();

struct FileEntry {
    int size = 0;
    string name;
    vector<unsigned long> blocks;
};

vector<FileEntry> files;
int blockNumber = -1;

int main(int argc, char *argv[]) {
    cerr << "Hello, World!" << endl;

    ifstream file;
    file.open(fileName);
    string inputLine;

    int cycle = 0;
    bool shouldContinue = true;

    createFile("N . 512");
    createFile("N .. 0");

    while (!file.eof() and shouldContinue) {

        if (cycle % HOW_OFTEN == 0) {
            print();
        }

        cout << "Getting line of input from file!" << endl;
        getline(file, inputLine);

        cout << "Got input from file! Selecting process method..." << endl;
        switch (inputLine.at(0)) {
            case 'C':
                cout << "Selected COPY method! Processing request..." << endl;
                copyFile(inputLine);
                break;

            case 'D':
                cout << "Selected DELETE method! Processing request..." << endl;
                deleteFile(inputLine);
                break;

            case 'N':
                cout << "Selected NEW method! Processing request..." << endl;
                createFile(inputLine);
                break;

            case 'M':
                cout << "Selected MODIFY method! Processing request..." << endl;
                modifyFile(inputLine);
                break;

            case 'R':
                cout << "Selected RENAME method! Processing request..." << endl;
                renameFile(inputLine);
                break;

            default:
                cout << "Selected END method! Terminating..." << endl;
                shouldContinue = false;
                break;
        }

        cycle++;
    }

    file.close();

    cerr << endl << "Program run complete!" << endl;

    print();

    return 0;
}

void createFile(const string &input) {
    char transaction;
    string name;
    int size;

    istringstream stream(input);
    stream >> transaction >> name >> size;

    for (const auto &i: files) {
        if (i.name == name) {
            cerr << "ERROR: Cannot create file with duplicate name!" << endl;
            return;
        }
    }

    FileEntry entry;
    entry.name = name;
    entry.size = size;

    int blockCount = entry.size / BLOCK_SIZE;

    int i = 1;
    while (i <= blockCount) {
        for (unsigned long j = 0; j < directory.size(); j++) {
            if (directory.at(j) == 0 and i == blockCount) {
                directory.at(j) = -1;
                blockNumber++;
                entry.blocks.push_back(j);
                i++;
                break;
            } else if (directory.at(j) == 0) {
                directory.at(j) = blockNumber;
                blockNumber++;
                entry.blocks.push_back(j);
                i++;
                break;
            }
        }
    }
    cerr << endl << "Created a new file named \"" << entry.name << "\" with size " << entry.size << "B!" << endl;
    files.push_back(entry);
}

void deleteFile(const string &input) {
    char transaction;
    string name;

    istringstream stream;
    stream >> transaction >> name;

    for (unsigned long i = 0; i < files.size(); i++) {
        if (files.at(i).name == name) {
            for (auto &j: files.at(i).blocks) {
                j = 0;
            }
            files.erase(files.begin() + i);
        }
    }

    cerr << endl << "Deleted a file named \"" << name << "\"!" << endl;
}

void copyFile(const string &input) {
    char transaction;
    string name;
    string newName;

    istringstream stream(input);
    stream >> transaction >> name >> newName;

    int size = 0;
    bool didFind = false;

    for (const auto &item: files) {
        if (item.name == name) {
            size = item.size;
            didFind = true;
        } else if (item.name == newName) {
            cerr << endl << "ERROR: Cannot copy file into file with same name!" << endl;
            return;
        }
    }

    if (didFind) {
        createFile("N " + newName + " " + to_string(size));
        cerr << "Copied existing file \"" << name << "\" to new file \"" << newName << "\"!" << endl;
    } else {
        cerr << endl << "ERROR: Cannot copy file that does not exist!" << endl;
    }
}

void renameFile(const string &input) {
    char transaction;

    string oldName;
    string newName;

    istringstream stream;
    stream >> transaction >> oldName >> newName;

    bool didFind = false;

    for (auto &i: files) {
        if (i.name == oldName) {
            didFind = true;
            i.name = newName;
            break;
        } else if (i.name == newName or oldName == newName) {
            cerr << endl << "ERROR: Tried to rename file to a name that already exists!" << endl;
            return;
        }
    }

    if (didFind) {
        cerr << endl << "Changed file name from \"" << oldName << "\" to \"" << newName << "\"!" << endl;
    } else {
        cerr << endl << "ERROR: No file named \"" << oldName << "\" was found! " << endl;
    }
}

void modifyFile(const string &input) {
    char transaction;
    string name;
    string number;

    istringstream stream;
    stream >> transaction >> name >> number;

    createFile("N " + name + "_copy " + number);
    deleteFile("D " + name);
    renameFile("R " + name + "_copy " + name);

    cerr << endl << "Modified file \"" << name << "\"!" << endl;
}

void print() {
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

        cerr << right << setw(2) << directory.at(i) << "\t";
        indent++;

        if (indent % 12 == 0) {
            cerr << endl;
        }
    }

    cerr << endl;
}
