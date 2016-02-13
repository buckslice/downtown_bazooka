#pragma once
#include <vector>

template <class T>
struct obj {
    obj() : id(-1) {}   // should init all objs to -1 im hoping
    obj(int id) : id(id) {}

    T data;
    int id; // index in the obj table (-1 if not being used)
};



// can also declare it like this
// template <class T, int N>
// to hardcode the size in during compile time
// seems pretty neat
template <class T>
class Pool {
public:
    Pool();
    Pool(int max_size);

    // get new obj
    int get();

    //T* getP();  // returns pointer to obj (safe since our vector will never resize)

    // get ptr to obj in pool
    obj<T>* getObj(int id);

    // get ptr to data in pool
    T* getData(int id);

    // returns obj back to pool
    void ret(int id);

    std::vector<obj<T>>& getObjects();

    size_t size() {
        return objs.size();
    }

private:
    std::vector<obj<T>> objs;
    std::vector<size_t> free_list;

};

template <class T>
Pool<T>::Pool() {
}

template <class T>
Pool<T>::Pool(int max_size) {
    objs.resize(max_size);
    for (size_t i = max_size; i > 0; i--) { // careful, size_t is unsigned
        free_list.push_back(i - 1);
    }
}

template <class T>
int Pool<T>::get() {
    if (free_list.empty()) {
        std::cout << "POOL EMPTY!!!" << std::endl;
        return -1;
    }
    size_t free = free_list.back();
    free_list.pop_back();
    objs[free].id = free;
    return free;
}

// pretty unsafe, just make sure to never return
// without updating everyone who references it
//template<class T>
//T* Pool<T>::getP() {
//    if (free_list.empty()) {
//        std::cout << "POOL EMPTY!!!" << std::endl;
//        return nullptr;
//    }
//    size_t free = free_list.back();
//    free_list.pop_back();
//    objs[free].id = free;
//    return &(objs[free].data);
//}

// get pointer to object in pool
template<class T>
obj<T>* Pool<T>::getObj(int id) {
    return objs[id].id == -1 ? nullptr : &objs[id];
}

// get pointer to data in pool
template<class T>
T* Pool<T>::getData(int id) {
    return objs[id].id == -1 ? nullptr : &objs[id].data;
}

// frees object at index
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

// ill do the iterator.... later...
// heres some nice examples to jog my barn
//http://www.cs.northwestern.edu/~riesbeck/programming/c++/stl-iterator-define.html#TOC8

//// iterator
//template <class T>
//class PoolIterator {
//private:
//    Pool<T>& pool;
//    int size;
//    int* p;
//public:
//    PoolIterator(Pool& pool, int size)
//        : pool{ pool }, size{ size } {
//    }
//
//    T& operator*() {
//        return pool.objs[p];
//    }
//
//};