#ifndef DELETEVALUE_HPP
#define DELETEVALUE_HPP

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <mutex>
#include <string>
#include <filesystem>

#include "../CustomStructures/MyVector.hpp"
#include "../CustomStructures/MyHashMap.hpp"

#include "../Other/JsonParser.hpp"

#include "../Other/Utilities.hpp"
#include "../structs.h"

#include "WhereValue.hpp"


using namespace std;

// перезапись во временный файл информации кроме удаленной
void removeData(MyVector<string>& namesOfTable, MyVector<string>& listOfCondition, SchemaInfo& dataOfShema) {
    Node* nodeWere = getConditionTree(listOfCondition);
     
    for (int i = 0; i < namesOfTable.length; i++) {
        int fileIndex = 1;
        string pathToCSV = dataOfShema.filepath + "/" + dataOfShema.name + "/" + namesOfTable.data[i];
        auto mutexIt = dataOfShema.tableMutexes.find(namesOfTable.data[i]);
        if (mutexIt != dataOfShema.tableMutexes.end()) {
            unique_lock<mutex> lock(mutexIt->second); // Блокировка мьютекса

            while (filesystem::exists(pathToCSV + "/" + to_string(fileIndex) + ".csv")) {
                ifstream file(pathToCSV + "/" + to_string(fileIndex) + ".csv");
                if (!file.is_open()) {
                    throw runtime_error("Ошибка открытия файла " + (pathToCSV + "/" + to_string(fileIndex) + ".csv"));
                }
                ofstream tempFile(pathToCSV + "/" + to_string(fileIndex) + "_temp.csv");

                string line;
                getline(file, line);
                tempFile << line;
                while (getline(file, line)) {
                    MyVector<string>* row = splitRow(line, ',');
                    try {
                        if (!isValidRow(nodeWere, *row, *dataOfShema.jsonStructure, namesOfTable.data[i])) {
                            tempFile << endl << line;
                        }
                    } catch (const exception& e) {
                        tempFile.close();
                        file.close();
                        remove((pathToCSV + "/" + to_string(fileIndex) + "_temp.csv").c_str());
                        throw runtime_error(e.what());
                        return;
                    }
                }
                tempFile.close();
                file.close();
                if (remove((pathToCSV + "/" + to_string(fileIndex) + ".csv").c_str()) != 0) {
                    throw runtime_error("Error deleting file");
                    return;
                }
                if (rename((pathToCSV + "/" + to_string(fileIndex) + "_temp.csv").c_str(), (pathToCSV + "/" + to_string(fileIndex) + ".csv").c_str()) != 0) {
                    throw runtime_error("Error renaming file");
                    return;
                }

                fileIndex++;
            }
        }
    }
}

// разбиение запроса удаления на кусочки
void parseDelete(const MyVector<string>& words, SchemaInfo& dataOfSchema) {
    MyVector<string>* namesOfTable = CreateVector<string>(5, 50);
    MyVector<string>* listOfCondition = CreateVector<string>(5, 50);
    int countTabNames = 0;
    int countWereData = 0;
    bool afterWhere = false;
    for (int i = 2; i < words.length; i++ ) {
        if (words.data[i][words.data[i].size() - 1] == ',') {
            words.data[i] = getSubsting(words.data[i], 0, words.data[i].size() - 1);
        }
        if (words.data[i] == "WHERE") {
            afterWhere = true;
        } else if (afterWhere) {
            AddVector<string>(*listOfCondition, words.data[i]);
            countWereData++;
        } else {
            countTabNames++;
            try {
                GetMap(*dataOfSchema.jsonStructure, words.data[i]);
            } catch (const exception& e) {
                throw runtime_error(e.what());
                return;
            }
            AddVector<string>(*namesOfTable, words.data[i]);
        }
    }
    if (countTabNames == 0 || countWereData == 0) {
        throw runtime_error("missing table name or data in WERE");
    }

    try {
        removeData(*namesOfTable, *listOfCondition, dataOfSchema);
    } catch (const exception& err) {
        throw;
        return;
    }
}


#endif // DELETEVALUE_HPP
