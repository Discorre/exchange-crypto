#ifndef VECTORDAS_H
#define VECTORDAS_H

#include <iostream>
#include <iomanip>


template <typename T>
struct MyVector {
    T* data;      //сам массив
    size_t length;        //длина
    size_t capacity;        //capacity - объем
    size_t LoadFactor; //с какого процента заполнения увеличиваем объем = 50%
};

template <typename T>
std::ostream& operator << (std::ostream& os, const MyVector<T>& vector) {
    for (size_t i = 0; i < vector.length; i++) {
        std::cout << vector.data[i];
        if (i < vector.length - 1) std::cout << std::setw(25);
    }
    return os;
}

template <typename T>
MyVector<T>* CreateVector(size_t initCapacity, size_t initLoadFactor) {
    if (initCapacity <= 0 || initLoadFactor <= 0 || initLoadFactor > 100) {
        throw std::runtime_error("Index out of range");
    }

    MyVector<T>* vector = new MyVector<T>;  // Создаем новый вектор
    vector->data = new T[initCapacity];  // Выделяем память под массив
    vector->length = 0;  // Инициализируем длину
    vector->capacity = initCapacity;  // Устанавливаем вместимость
    vector->LoadFactor = initLoadFactor;  // Устанавливаем фактор загрузки
    return vector;
}

// увеличение массива
template <typename T>
void Expansion(MyVector<T>& vector) {
    size_t newCap = vector.capacity * 2;
    T* newData = new T[newCap];
    for (size_t i = 0; i < vector.length; i++) {     //копируем данные из старого массива в новый
        newData[i] = vector.data[i];
    }
    delete[] vector.data;                      // очистка памяти
    vector.data = newData;
    vector.capacity = newCap;
}

// добавление элемента в вектор
template <typename T>
void AddVector(MyVector<T>& vector, T value) {
    if ((vector.length + 1) * 100 / vector.capacity >= vector.LoadFactor) { //обновление размера массива
        Expansion(vector);
    }
    vector.data[vector.length] = value;
    vector.length++;
}


//удаление элемента из вектора
template <typename T>
void DeleteVector(MyVector<T>& vector, size_t index) {
    if (index < 0 || index >= vector.length) {
        throw std::runtime_error("Index out of range");
    }

    for (size_t i = index; i < vector.length - 1; i++) {
        vector.data[i] = vector.data[i + 1];
    }

    vector.length--;
}


// замена элемента по индексу
template <typename T>
void ReplaceVector(MyVector<T>& vector, size_t index, T value) {
    if (index < 0 || index >= vector.length) {
        throw std::runtime_error("Index out of range");
    }
    vector.data[index] = value;
}

#endif