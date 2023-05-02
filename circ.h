#pragma once
#include <vector>
template<typename T>
class CircBuf {
private:
    std::vector<T> container;
    int capacity;
public:
    CircBuf(int _size) : capacity(_size) {};
    void add_elements(T* elements, int count);
    std::vector<T>* get_buffer() { return &container; }
    bool is_ready() { return container.size() == capacity; }
};

