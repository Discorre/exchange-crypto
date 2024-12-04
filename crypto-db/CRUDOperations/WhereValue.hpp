#ifndef WHEREVALUE_HPP
#define WHEREVALUE_HPP

#include <iostream>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <string>
#include "../CustomStructures/MyVector.hpp"
#include "../Other/json.hpp"
#include "../CRUDOperations/WhereValue.hpp"
#include "../structs.h"

#include "../Other/JsonParser.hpp"
#include "../Other/Utilities.hpp"

using namespace std;

string SanitizeText(string str) {
    if (str[0] == '\'' && str[str.size() - 1] == '\'') {
        str = getSubsting(str, 1, str.size() - 1);
        return str;
    } else {
        throw runtime_error("Неверный синтаксис в WHERE " + str);
    }
}




bool isValidRow(Node* node, const MyVector<string>& row, const MyHashMap<string, MyVector<string>*>& jsonStructure, const string& namesOfTable) {
    if (!node) {
        return false;
    }

    switch (node->nodeType) {
    case NodeType::ConditionNode: {
        if (node->value.length != 3) {
            return false;
        }

        MyVector<string> *part1Splitted = splitRow(node->value.data[0], '.');
        if (part1Splitted->length != 2) {
            return false;
        }
    
        // существует ли запрашиваемая таблица
        int columnIndex = -1;
        try {
            MyVector<string>* colNames = GetMap(jsonStructure, part1Splitted->data[0]);
            for (int i = 0; i < colNames->length; i++) {
                if (colNames->data[i] == part1Splitted->data[1]) {
                    columnIndex = i;
                    break;
                }
            }
        } catch (const exception& e) {
            throw runtime_error(e.what());
            return false;
        }

        if (columnIndex == -1) {
            cerr << "Column " << part1Splitted->data[1] << " is missing in table " << part1Splitted->data[0] << std::endl;
            return false;
        }

        string delApostr = SanitizeText(node->value.data[2]);
        if (namesOfTable == part1Splitted->data[0] && row.data[columnIndex] == delApostr) {  
            return true;
        }

        return false;
    }
    case NodeType::OrNode:
        return isValidRow(node->left, row, jsonStructure, namesOfTable) ||
                isValidRow(node->right, row, jsonStructure, namesOfTable);
    case NodeType::AndNode:
        return isValidRow(node->left, row, jsonStructure, namesOfTable) &&
                isValidRow(node->right, row, jsonStructure, namesOfTable);
    default:
        return false;
    }
}

bool writeAllRows(Node* nodeWere, const string& nameOfTable , string& line, MyVector<MyVector<string>*>& tabData, SchemaInfo& schemaData, bool whereValue) {
    MyVector<string>* row = splitRow(line, ',');
    if (whereValue) {
        try {
            if (isValidRow(nodeWere, *row, *schemaData.jsonStructure, nameOfTable)) {
                AddVector(tabData, row);
            }
        } catch (const exception& err) {
            throw;
            return false;
        }
    } else {
        AddVector(tabData, row);
    }
    return true;
}

// считывание подходящих строк из выбранных столбцов
bool writePhRows(Node* nodeWere, const string& nameOfTable, string& line, MyVector<MyVector<string>*>& tabData, SchemaInfo& dataOfSchema, bool whereValue, MyVector<int>& colIndex) {
    MyVector<string>* row = splitRow(line, ',');
    MyVector<string>* newRow = CreateVector<string>(colIndex.length, 50);
    if (whereValue) {
        try {
            if (isValidRow(nodeWere, *row, *dataOfSchema.jsonStructure, nameOfTable)) {
                for (int i = 0; i < colIndex.length; i++) {
                    AddVector(*newRow, row->data[colIndex.data[i]]);
                }
                AddVector(tabData, newRow);
            }
        } catch (const exception& err) {
            throw;
            return false;
        }
    } else {
        for (int i = 0; i < colIndex.length; i++) {
            AddVector(*newRow, row->data[colIndex.data[i]]);
        }
        AddVector(tabData, newRow);
    }
    return true;
}



// Вспомогательная функция для разделения строки по оператору
MyVector<MyVector<string>*>* splitByOperator(const MyVector<string>& query, const string& op) {
    MyVector<string>* left = CreateVector<string>(6, 50);
    MyVector<string>* right = CreateVector<string>(6, 50);
    bool afterOp = false;
    for (int i = 0; i < query.length; i++) {
        if (query.data[i] == op) {
            afterOp = true;
        } else if (afterOp) {
            AddVector(*right, query.data[i]);
        } else {
            AddVector(*left, query.data[i]);
        }
    }
    MyVector<MyVector<string>*>* parseVector = CreateVector<MyVector<string>*>(5, 50);
    if (afterOp) {
        AddVector(*parseVector, left);
        AddVector(*parseVector, right);
        
    } else {
        AddVector(*parseVector, left);
    }
    return parseVector;
}


Node* getConditionTree(const MyVector<string>& query) {
    MyVector<MyVector<string>*>* orParts = splitByOperator(query, "OR");

    // Если найден оператор OR
    if (orParts->length > 1) {
        Node* root = new Node(NodeType::OrNode);
        root->left = getConditionTree(*orParts->data[0]);
        root->right = getConditionTree(*orParts->data[1]);
        return root;
    }
    // Если найден оператор AND
    MyVector<MyVector<std::string>*>* andParts = splitByOperator(query, "AND");
    if (andParts->length > 1) {
        Node* root = new Node(NodeType::AndNode);
        root->left = getConditionTree(*andParts->data[0]);
        root->right = getConditionTree(*andParts->data[1]);
        return root;
    }

    // Если это простое условие
    return new Node(NodeType::ConditionNode, query);
}



#endif // WHEREVALUE_HPP