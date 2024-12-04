#ifndef SELECTVALUE_HPP
#define SELECTVALUE_HPP

#include <iostream>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <string>
#include <arpa/inet.h>

#include "../CustomStructures/MyVector.hpp"
#include "../CustomStructures/MyHashMap.hpp"

#include "../structs.h"

#include "../Other/JsonParser.hpp"
#include "../Other/Utilities.hpp"

#include "WhereValue.hpp"

using namespace std;

// чтение таблицы из файла
MyVector<MyVector<string>*>* ReadTable(const string& nameOfTable, SchemaInfo& dataOfSchema, const MyVector<string>& namesOfColumns, const MyVector<string>& listOfCondition, bool whereValue) {
    MyVector<MyVector<string>*>* tabData = CreateVector<MyVector<string>*>(5, 50);
    string pathToCSV = dataOfSchema.filepath + "/" + dataOfSchema.name + "/" + nameOfTable;
    int fileIndex = 1;

    // Захватываем мьютекс для таблицы, если она существует в tableMutexes
    auto mutexIt = dataOfSchema.tableMutexes.find(nameOfTable);
    if (mutexIt != dataOfSchema.tableMutexes.end()) {
        unique_lock<mutex> lock(mutexIt->second); // Блокировка мьютекса
        
        Node* nodeWere = getConditionTree(listOfCondition);
        while (filesystem::exists(pathToCSV + "/" + to_string(fileIndex) + ".csv")) {
            ifstream file(pathToCSV + "/" + to_string(fileIndex) + ".csv");
            if (!file.is_open()) {
                throw runtime_error("Ошибка открытия файла: " + (pathToCSV + "/" + to_string(fileIndex) + ".csv"));
            }
            string firstLine;
            getline(file, firstLine);
            if (namesOfColumns.data[0] == "*") {
                string line;
                while (getline(file, line)) {
                    if (!writeAllRows(nodeWere, nameOfTable, line, *tabData, dataOfSchema, whereValue)) {
                        file.close();
                        return tabData;
                    }
                }
            } else {
                MyVector<string>* filenamesOfColumns = GetMap<string, MyVector<string>*>(*dataOfSchema.jsonStructure, nameOfTable);
                MyVector<int>* colIndex = CreateVector<int>(10, 50);
                for (int i = 0; i < filenamesOfColumns->length; i++) {
                    for (int j = 1; j < namesOfColumns.length; j++) {
                        if (filenamesOfColumns->data[i] == namesOfColumns.data[j]) {
                            AddVector(*colIndex, i);
                        }
                    }
                }
                string line;
                while (getline(file, line)) {
                    if (!writePhRows(nodeWere, nameOfTable, line, *tabData, dataOfSchema, whereValue, *colIndex)) {
                        file.close();
                        return tabData;
                    }
                }
            }

            file.close();
            fileIndex += 1;
        }
    }
    return tabData;
}


// вывод содержимого таблиц в виде декартового произведения
void cartesianProduct(const MyVector<MyVector<MyVector<string>*>*>& dataOfTables, MyVector<MyVector<string>*>& temp, int counterTab, int tab, int clientSocket) {
    for (int i = 0; i < dataOfTables.data[counterTab]->length; i++) {
        temp.data[counterTab] = dataOfTables.data[counterTab]->data[i];

        if (counterTab < tab - 1) {
            cartesianProduct(dataOfTables, temp, counterTab + 1, tab, clientSocket);
        } else {
            for (int j = 0; j < tab; j++) {
                for (int k = 0; k < temp.data[j]->length; k++) {
                    send(clientSocket, (temp.data[j]->data[k] + " ").c_str(), (temp.data[j]->data[k] + " ").size(), 0);
                }
            }
            string enter = "\n";
            send(clientSocket, enter.c_str(), enter.size(), 0);
        }
    }

    return;
}

// подготовка к чтению и выводу данных
void selectDataPreparation(const MyVector<string>& namesOfColumns, const MyVector<string>& namesOfTables, const MyVector<string>& listOfCondition, SchemaInfo& dataOfSchema, bool whereValue, int clientSocket) {
    MyVector<MyVector<MyVector<string>*>*>* dataOfTables = CreateVector<MyVector<MyVector<string>*>*>(10, 50);
    if (namesOfColumns.data[0] == "*") {      // чтение всех данных из таблиц
        for (int j = 0; j < namesOfTables.length; j++) {
            MyVector<MyVector<string>*>* tableData = ReadTable(namesOfTables.data[j], dataOfSchema, namesOfColumns, listOfCondition, whereValue);
            AddVector(*dataOfTables, tableData);
        }
    } else {
        for (int i = 0; i < namesOfTables.length; i++) {
            MyVector<string>* tabColPair = CreateVector<string>(5, 50);
            AddVector(*tabColPair, namesOfTables.data[i]);
            for (int j = 0; j < namesOfColumns.length; j++) {
                MyVector<string>* splitNamesOfColumns = splitRow(namesOfColumns.data[j], '.');
                try {
                    GetMap(*dataOfSchema.jsonStructure, splitNamesOfColumns->data[0]);
                } catch (const exception& e) {
                    throw runtime_error(e.what());
                    return;
                }
                if (splitNamesOfColumns->data[0] == namesOfTables.data[i]) {
                    AddVector(*tabColPair, splitNamesOfColumns->data[1]);
                }
            }
            MyVector<MyVector<string>*>* tableData = ReadTable(tabColPair->data[0], dataOfSchema, *tabColPair, listOfCondition, whereValue);;
            AddVector(*dataOfTables, tableData);
        }
    }

    MyVector<MyVector<string>*>* temp = CreateVector<MyVector<string>*>(dataOfTables->length * 2, 50);
    string resStr;
    cartesianProduct(*dataOfTables, *temp, 0, dataOfTables->length, clientSocket);
    return;
}

// парсинг SELECT запроса
void parseSelect(const MyVector<string>& slovs, SchemaInfo& dataOfSchema, int clientSocket) {
    MyVector<string>* namesOfColumns = CreateVector<string>(10, 50);          // названия колонок в формате таблица1.колонка1
    MyVector<string>* namesOfTables = CreateVector<string>(10, 50);        // названия таблиц в формате  таблица1
    MyVector<string>* listOfCondition = CreateVector<string>(10, 50);     // список условий where
    bool afterFrom = false;
    bool afterWhere = false;
    int countTabNames = 0;
    int countData = 0;
    int countWhereData = 0;
    for (int i = 1; i < slovs.length; i++) {
        if (slovs.data[i][slovs.data[i].size() - 1] == ',') {
            slovs.data[i] = getSubsting(slovs.data[i], 0, slovs.data[i].size() - 1);
        }
        if (slovs.data[i] == "FROM") {
            afterFrom = true;
        } else if (slovs.data[i] == "WHERE") {
            afterWhere = true;
        } else if (afterWhere) {
            countWhereData++;
            AddVector<string>(*listOfCondition, slovs.data[i]);
        } else if (afterFrom) {
            try {
                GetMap(*dataOfSchema.jsonStructure, slovs.data[i]);
            } catch (const exception& e) {
                throw runtime_error(e.what());
                return;
            }
            countTabNames++;
            AddVector(*namesOfTables, slovs.data[i]);
        } else {
            countData++;
            AddVector(*namesOfColumns, slovs.data[i]);
        }
    }
    if (countTabNames == 0 || countData == 0) {
        throw runtime_error("Отсутствует имя таблицы или данные в FROM");
    }
    if (countWhereData == 0) {
        selectDataPreparation(*namesOfColumns, *namesOfTables, *listOfCondition, dataOfSchema, false, clientSocket);
    } else {
        selectDataPreparation(*namesOfColumns, *namesOfTables, *listOfCondition, dataOfSchema, true, clientSocket);
    }
}

#endif // SELECTVALUE_HPP