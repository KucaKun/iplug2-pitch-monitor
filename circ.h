#pragma once
#include <vector>
template<typename T>
class CircBuf {
private:
    std::vector<T> container;
    int capacity;
public:
    CircBuf(int _size) : capacity(_size) {};
    void add_elements(T* elements, int count) {
        for (int i = 0; i < count; i++) {
            container.push_back(elements[i]);
        }
        if (container.size() > capacity) {
            int offset = container.size() - capacity;
            std::vector<T> tmp = container;
            container.clear();
            auto it = tmp.begin();
            std::advance(it, offset);
            container.insert(container.begin(), it, tmp.end());
        }
    }
    std::vector<T>* get_buffer() { return &container; }
    bool is_ready() { return container.size() == capacity; }
};

