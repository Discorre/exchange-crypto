#ifndef MAPDAS_H
#define MAPDAS_H

#include <iostream>
#include <string>


// структура для хранения значения
template <typename TK, typename TV>
struct NodeMap {
    TK key;
    TV value;
    NodeMap* next;
};

// структура для хранения ключа и значения
template <typename TK, typename TV>
struct MyHashMap {
    NodeMap<TK, TV>** data;
    size_t length;
    size_t capacity;
    size_t loadFactor;
};

// хэш-функция для ключа string
template <typename TK>
int HashCode(const TK& key, const int capacity) {
    unsigned long hash = 5381;
    int c = 0;
    for (char ch : key) {
        hash = ((hash << 5) + hash) + ch;
    }
    return hash % capacity;
}

// инициализация хэш таблицы
template <typename TK, typename TV>
MyHashMap<TK, TV>* CreateMap(int initCapacity, int initLoadFactor) {
    if (initCapacity <= 0 || initLoadFactor <= 0 || initLoadFactor > 100) {
        throw std::runtime_error("Индекс вне диапазона");
    }

    MyHashMap<TK, TV>* map = new MyHashMap<TK, TV>;
    map->data = new NodeMap<TK, TV>*[initCapacity];
    for (size_t i = 0; i < initCapacity; i++) {
        map->data[i] = nullptr;
    }

    map->length = 0;
    map->capacity = initCapacity;
    map->loadFactor = initLoadFactor;
    return map;
}

// расширение
template <typename TK, typename TV>
void Expansion(MyHashMap<TK, TV>& map) {
    size_t newCap = map.capacity * 2;
    NodeMap<TK, TV>** newData = new NodeMap<TK, TV>*[newCap];
    for (size_t i = 0; i < newCap; i++) {
        newData[i] = nullptr;
    }
    // проход по всем ячейкам
    for (size_t i = 0; i < map.capacity; i++) {
        NodeMap<TK, TV>* curr = map.data[i];
        // проход по парам коллизионных значений и обновление
        while (curr != nullptr) {
            NodeMap<TK, TV>* next = curr->next;
            size_t index = HashCode(curr->key, newCap);
            curr->next = newData[index];
            newData[index] = curr;
            curr = next;
        }
    }

    delete[] map.data;

    map.data = newData;
    map.capacity = newCap;
}

// обработка коллизий
template <typename TK, typename TV>
void CollisionManage(MyHashMap<TK, TV>& map, int index, const TK& key, const TV& value) {
    NodeMap<TK, TV>* newNode = new NodeMap<TK, TV>{key, value, nullptr};
    NodeMap<TK, TV>* curr = map.data[index];
    while (curr->next != nullptr) {
        curr = curr->next;
    }
    curr->next = newNode;
}

// добавление элементов
template <typename TK, typename TV>
void AddMap(MyHashMap<TK, TV>& map, const TK& key, const TV& value) {
    if ((map.length + 1) * 100 / map.capacity >= map.loadFactor) {
        Expansion(map);
    }
    size_t index = HashCode(key, map.capacity);
    NodeMap<TK, TV>* temp = map.data[index];
    if (temp != nullptr) {
        while (temp != nullptr) {
            if (temp->key == key) {
                // Элемент уже существует, обновить значение
                temp->value = value;
                map.data[index] = temp;
                return;
            }
            temp = temp->next;
        }
        CollisionManage(map, index, key, value);
    } else {
        NodeMap<TK, TV>* newNode = new NodeMap<TK, TV>{key, value, map.data[index]};
        map.data[index] = newNode;
        map.length++;
    }

}

// поиск элементов по ключу
template <typename TK, typename TV>
TV GetMap(const MyHashMap<TK, TV>& map, const TK& key) {
    size_t index = HashCode(key, map.capacity);
    NodeMap<TK, TV>* curr = map.data[index];
    while (curr != nullptr) {
        if (curr->key == key) {
            return curr->value;
        }
        curr = curr->next;
    }

    throw;
}


// удаление элементов
template <typename TK, typename TV>
void DeleteMap(MyHashMap<TK, TV>& map, const TK& key) {
    size_t index = HashCode(key, map.capacity);
    NodeMap<TK, TV>* curr = map.data[index];
    NodeMap<TK, TV>* prev = nullptr;
    while (curr != nullptr) {
        if (curr->key == key) {
            if (prev == nullptr) {
                map.data[index] = curr->next;
            } else {
                prev->next = curr->next;
            }
            delete curr;
            map.length--;
            return;
        }
        prev = curr;
        curr = curr->next;
    }
    throw;
}


// очистка памяти
template <typename TK, typename TV>
void DestroyMap(MyHashMap<TK, TV>& map) {
    for (size_t i = 0; i < map.capacity; i++) {
        NodeMap<TK, TV>* curr = map.data[i];
        while (curr != nullptr) {
            NodeMap<TK, TV>* next = curr->next;
            delete curr;
            curr = next;
        }
    }
    delete[] map.data;
    map.data = nullptr;
    map.length = 0;
    map.capacity = 0;
}


#endif
