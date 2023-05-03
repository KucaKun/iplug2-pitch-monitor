//template <typename T> void CircBuf<T>::add_elements(T* elements, int count) {
//    for (int i = 0; i < count; i++) {
//        container.push_back(elements[i]);
//    }
//    if (container.size() > capacity) {
//        int offset = container.size() - capacity;
//        std::vector<T> tmp = container;
//        container.clear();
//        container.insert(container.begin(), capacity, tmp.begin() + offset);
//    }
//}
