#pragma once
#include <vector>
#include <cassert>

//http://www.codeproject.com/Articles/746630/O-Object-Pool-in-Cplusplus
// class that pools memory
// TODO: try redoing to be a little more like https://sites.google.com/site/initupdatedraw/home
// ie: add indices and bit shifting to calculate stuff rather than pointer arithmetic
// could also try making this pool resizable like in original link
template <class T>
class MemPool {
public:
    struct Item {
        T data;
        bool free;
    };

    explicit MemPool(size_t max) :
        maxCount(max),
        nextFree(nullptr),
        count(0) {
        if (maxCount > 64000) { // cap at arbitrary 64K
            maxCount = 64000;
        }
        // operator new to allocate a whole block of memory for this pool
        memory = (char*)::operator new(maxCount * itemSize);
    }

    ~MemPool() {
        ::operator delete(memory);  // deallocate whole memory block
    }

    // constructs data and returns pointer
    T* alloc() {
        if (nextFree) { // if nextFree is not null
            Item* ret = nextFree;   // get pointer to item at nextFree
            nextFree = *((Item**)nextFree); // assign nextFree to be whatever was in this free spot
            new(ret) Item();    // placement new 
            return &ret->data;
        }

        // if there is no more room and nextFree is null then return nullptr
        // later add ability to allocate new memory blocks like original article
        if (count >= maxCount) {
            std::cout << "WARNING::MEMPOOL::EMPTY: of " << typeid(T).name() << std::endl;
            return nullptr;
        }
        // else increment count and allocate
        Item* ret = new(memory + itemSize * count) Item();
        ++count;
        return &ret->data;
    }

    // frees data from pointer address
    void free(T* data) {
        data->~T(); //call destructor on data

        // cast data as a pointer to a pointer
        // and store the current location of nextFree in its place
        *((Item**)data) = nextFree;
        nextFree = (Item*)data; // make nextFree point to this item
        nextFree->free = true;  // set the free bool so next() will skip it
    }

    // frees data from index
    void free(int index) {
        free((T*)(memory + index*itemSize));
    }

    // frees all allocated data in pool
    void freeAll() {
        // call destructors on all allocated data
        for (T* t = nullptr; next(t);) {
            t->~T();
        }
        // reset count and nextFree
        count = 0;
        nextFree = nullptr;
    }

    // this is pretty much an iterator
    // call like so to iterate over all allocated elements:
    // for(T* t = nullptr; pool.next(t);){}
    bool next(T*& data) {   // reference to a pointer
        char* address = (char*)data;
        if (!data) {    // if data is nullptr then start at one back from beginning
            address = memory - itemSize;
        }
        do {    // increment forward and check to make sure not past memory block
            address += itemSize;
            if (address >= memory + itemSize * count) {
                data = nullptr;
                return false;
            }
        } while (((Item*)(address))->free); // keep doing so while the item is free
        // update the pointer reference with address of next allocated
        data = (T*)address;
        return true;
    }

    int getIndex(T* data) { // get index from type pointer
        return ((char*)data - memory) / itemSize;
    }

    int getIndex(char* ptr) {   // get index to current item from random point in memory block
        return (int)(((char*)ptr - memory) / itemSize);
    }

    T* get(int index) { // return data pointer by index
        return (T*)(memory + index * itemSize);
    }

private:
    char* memory;    // block of memory for this pool
    size_t maxCount; // number of elements in pool

    Item* nextFree;  // pointer to next free element
    size_t count;    // signifies highest element index since reset

    static const size_t itemSize;
};

// ensures itemSize is at least 8 bytes
// needs to be able to store the pointer for free list and the bool to signify free
template<class T>
const size_t MemPool<T>::itemSize = sizeof(Item) < 8 ? 8 : sizeof(Item);