#ifndef INSERTVALUE_HPP
#define INSERTVALUE_HPP

#include <iostream>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <string>

#include "../CustomStructures/MyVector.hpp"
#include "../CustomStructures/MyHashMap.hpp"

#include "../Other/JsonParser.hpp"
#include "../Other/Utilities.hpp"

using namespace std;

// удаление опострафа и проверка синтаксиса
string cleanText(string& str) {
    if (str[str.size() - 1] == ',' && str[str.size() - 2] == ')') {
        str = getSubsting(str, 0, str.size() - 2);
    } else if (str[str.size() - 1] == ',' || str[str.size() - 1] == ')') {
        str = getSubsting(str, 0, str.size() - 1);
    }

    if (str[0] == '\'' && str[str.size() - 1] == '\'') {
        str = getSubsting(str, 1, str.size() - 1);
        return str;
    } else {
        throw runtime_error("invalid sintaxis in VALUES " + str);
    }
}

// проверка количества аргументов относительно столбцов таблиц
void Validate(int colLen, const MyVector<string>& namesOfTable, const MyHashMap<string, MyVector<string>*>& jsonStructure) {
    for (int i = 0; i < namesOfTable.length; i++) {
        MyVector<string>* temp = GetMap<string, MyVector<string>*>(jsonStructure, namesOfTable.data[i]);
        if (temp->length - 1 != colLen) {      // добавить удаление первого элемента из мапа
            throw runtime_error("the number of arguments is not equal to the columns in " + namesOfTable.data[i]);
        }
    }
}

// чтение файла с количеством записей и перезапись
int readPrKey(const string& path, const bool record, const int newID) {
    fstream pkFile(path);
    if (!pkFile.is_open()) {
        throw runtime_error("Не удалось открыть" + path);
    }
    int lastID = 0;
    if (record) {
        pkFile << newID;
    } else {
        pkFile >> lastID;
    }
    pkFile.close();
    return lastID;
}

// добавление строк в файл
void insertRows(MyVector<MyVector<string>*>& addNewData, MyVector<string>& namesOfTable, SchemaInfo& dataOfSchema) {
    for (int i = 0; i < namesOfTable.length; i++) {
        string pathToCSV = dataOfSchema.filepath + "/" + dataOfSchema.name + "/" + namesOfTable.data[i];
        int lastID = 0;

        // Захватываем мьютекс для таблицы, если она существует в tableMutexes
        auto mutexIt = dataOfSchema.tableMutexes.find(namesOfTable.data[i]);
        if (mutexIt != dataOfSchema.tableMutexes.end()) {
            unique_lock<mutex> lock(mutexIt->second); // Блокировка мьютекса
            cout << "mutex is locked " << namesOfTable.data[i] << endl;

            try {
                lastID = readPrKey(pathToCSV + "/" + namesOfTable.data[i] + "_pk_sequence.txt", false, 0);
            } catch (const exception& err) {
                throw runtime_error(err.what());
                return;
            }

            int newID = lastID;
            for (int j = 0; j < addNewData.length; j++) {
                newID++;
                string tempPath;
                if (lastID / dataOfSchema.tuplesLimit < newID / dataOfSchema.tuplesLimit) {
                    tempPath = pathToCSV + "/" + to_string(newID / dataOfSchema.tuplesLimit + 1) + ".csv";
                } else {
                    tempPath = pathToCSV + "/" + to_string(lastID / dataOfSchema.tuplesLimit + 1) + ".csv";
                }
                fstream csvFile(tempPath, ios::app);
                if (!csvFile.is_open()) {
                    throw runtime_error("Failed to open" + tempPath);
                }
                csvFile << endl << newID;
                for (int k = 0; k < addNewData.data[j]->length; k++) {
                    csvFile << "," << addNewData.data[j]->data[k];
                }
                csvFile.close();
            }
            readPrKey(pathToCSV + "/" + namesOfTable.data[i] + "_pk_sequence.txt", true, newID);
        }
    }
}

// разделение запроса вставки на части
void parseInsert(const MyVector<string>& slovs, SchemaInfo& dataOfSchema) {
    MyVector<string>* namesOfTables = CreateVector<string>(5, 50);
    MyVector<MyVector<string>*>* addNewData = CreateVector<MyVector<string>*>(10, 50);
    bool afterValues = false;
    int countTabNames = 0;
    int countAddData = 0;
    for (int i = 2; i < slovs.length; i++) {
        if (slovs.data[i][slovs.data[i].size() - 1] == ',') {
            slovs.data[i] = getSubsting(slovs.data[i], 0, slovs.data[i].size() - 1);
        }
        if (slovs.data[i] == "VALUES") {
            afterValues = true;
        } else if (afterValues) {
            countAddData++;
            if (slovs.data[i][0] == '(') {
                MyVector<string>* tempData = CreateVector<string>(5, 50);
                slovs.data[i] = getSubsting(slovs.data[i], 1, slovs.data[i].size());

                while (slovs.data[i][slovs.data[i].size() - 1] != ')' && slovs.data[i][slovs.data[i].size() - 2] != ')') {
                    try {
                        cleanText(slovs.data[i]);
                    } catch (const exception& err) {
                        throw runtime_error(err.what());
                        return;
                    }
                    
                    AddVector<string>(*tempData, slovs.data[i]);
                    i++;
                }
                try {
                    cleanText(slovs.data[i]);
                    AddVector<string>(*tempData, slovs.data[i]);
                    Validate(tempData->length, *namesOfTables, *dataOfSchema.jsonStructure);
                } catch (const exception& err) {
                    throw runtime_error(err.what());
                    return;
                }
                AddVector<MyVector<string>*>(*addNewData, tempData);
            }
            
        } else {
            countTabNames++;
            try {
                GetMap(*dataOfSchema.jsonStructure, slovs.data[i]);
            } catch (const exception& err) {
                throw runtime_error(err.what());
                return;
            }
            AddVector<string>(*namesOfTables, slovs.data[i]);
        }
    }
    if (countTabNames == 0 || countAddData == 0) {
        throw runtime_error("missing table name or data in VALUES");
    }

    try {
        insertRows(*addNewData, *namesOfTables, dataOfSchema);
    } catch (const exception& err) {
        throw runtime_error(err.what());
        return;
    }
}

#endif // INSERTVALUE_HPP