#pragma once

#include <string>
#include <mutex>
#include <map>

#include "CustomStructures/MyHashMap.hpp"
#include "CustomStructures/MyVector.hpp"

using namespace std;

struct SchemaInfo {
    string filepath = ".";
    string name;
    int tuplesLimit;
    MyHashMap<string, MyVector<string>*>* jsonStructure;
    map<string, mutex> tableMutexes;
};

enum class NodeType {
    ConditionNode,
    OrNode,
    AndNode
};

// Структура
struct Node {
    NodeType nodeType;
    MyVector<std::string> value;
    Node* left;
    Node* right;

    Node(NodeType type, const MyVector<std::string> val = {}, Node* l = nullptr, Node* r = nullptr)
        : nodeType(type), value(val), left(l), right(r) {}
};