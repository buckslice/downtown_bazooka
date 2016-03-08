#pragma once
#include <vector>
#include <cassert>

template <class T>
struct obj {
    obj() : id(-1) {}   // should init all objs to -1
    obj(int id) : id(id) {}

    T data;
    int id; // index in the obj table (-1 if not being used)
};

// can also declare it like this
// template <class T, int N>
// to hardcode the size in during compile time
// seems pretty neat

// class that pools things
// if you use the return function of this class make sure things are being referenced
// by the int lookups (and check if >= 0 before using) rather than pointers
template <class T>
class Pool {
public:
    Pool();
    Pool(int max_size);

    // get new obj, return int index to save and look up later
    int get();

    // get ptr to obj in pool
    obj<T>* getObj(int id);

    // get ptr to data in pool
    T* getData(int id);

    // returns obj back to pool
    void ret(int id);

    // returns all objects in pool
    void returnAll();

    // get internal object list from pool (make sure you declare as reference)
    std::vector<obj<T>>& getObjects();
    std::vector<size_t>& getFreeList();

    // returns max size
    size_t getSize() {
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
        //std::cout << "POOL EMPTY!!! WEEEEEEEEEEE";
        // maybe should assert here instead, or just no message?
        return -1;
    }
    size_t free = free_list.back();
    free_list.pop_back();
    objs[free].id = free;
    return free;
}

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
    // assert crashes the program if the boolean statement evaluates false
    // only works in debug mode and you can #define NDEBUG to skip it in that file (or maybe whole project, not sure)
    assert(id >= 0 && id < objs.size());
    objs[id].id = -1;
    free_list.push_back(id);
}

// should probs make an iterator
template <class T>
std::vector<obj<T>>& Pool<T>::getObjects() {
    return objs;
}

template <class T>
std::vector<size_t>& Pool<T>::getFreeList() {
    return free_list;
}

template <class T>
void Pool<T>::returnAll() {
    for (size_t i = 0, len = objs.size(); i < len; ++i) {
        if (objs[i].id < 0) {
            continue;
        }
        ret(i);
    }
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