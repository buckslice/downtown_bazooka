#pragma once
#include <vector>

template <class T>
struct obj {
    obj() : id(-1) {}   // should init all objs to -1 im hoping
    obj(int id) : id(id) {}

    T data;
    int id; // index in the obj table (-1 if not being used)
};

template <class T>
class Pool {
public:
    Pool();
    Pool(int max_size);

    // get new obj
    int get();

    // get ptr to obj in pool
    obj<T>* get(int id);

    // returns obj back to pool
    void ret(int id);

    std::vector<obj<T>>& getObjects();

private:
    std::vector<obj<T>> objs;
    std::vector<size_t> free_list;

};

template <class T>
Pool<T>::Pool() {
}

template <class T>
Pool<T>::Pool(int max_size) {
    //objs.resize(max_size);
    for (size_t i = max_size - 1; i >= 0; i--) {
        free_list.push_back(i);
    }
}

template <class T>
int Pool<T>::get() {
    if (free_list.empty()) {
        return -1;
    }
    size_t free = free_list.back();
    free_list.pop_back();
    objs[free].id = free;
    return free;
}

template<class T>
obj<T>* Pool<T>::get(int id) {
    return objs[id].id == -1 ? nullptr : &objs[i];
}

template <class T>
void Pool<T>::ret(int id) {
    objs[id].id = -1;
    free_list.push_back(id);
}

// should probs make an iterator
template <class T>
std::vector<obj<T>>& Pool<T>::getObjects() {
    return objs;
}
