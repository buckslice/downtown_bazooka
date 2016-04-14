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

    explicit MemPool(size_t max_size) :
        max_size(max_size),
        nextFree(nullptr),
        count(0) {
        if (max_size > 64000) { // cap at arbitrary 64K
            max_size = 64000;
        }
        // operator new to allocate a whole block of memory for this pool
        memory = (char*)::operator new(max_size * itemSize);
    }

    ~MemPool() {
        ::operator delete(memory);  // deallocate whole memory block
    }

    T* alloc() {
        if (nextFree) { // if nextFree is not null
            Item* ret = nextFree;   // get pointer to item at nextFree
            nextFree = *((Item**)nextFree); // assign nextFree to be whatever was in this free spot
            new(ret) Item();    // placement new 
            return &ret->data;
        }

        if (count >= max_size) {
            return nullptr; // later allocate new memory block
        }
        // else increment count and allocate
        Item* ret = new(memory + itemSize * count) Item();
        ++count;
        return &ret->data;
    }

    void free(T* data) {
        data->~T(); //call destructor on data

                    // cast data as a pointer to a pointer
                    // and store the current location of nextFree in its place
        *((Item**)data) = nextFree;
        nextFree = (Item*)data; // make nextFree point to this item
        nextFree->free = true;
    }

    void free(int index) {
        free((T*)(memory + index*itemSize));
    }

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
        do {
            address += itemSize;
            if (address >= memory + itemSize * count) {
                data = nullptr;
                return false;
            }
        } while (((Item*)(address))->free);

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
    char* memory;
    size_t max_size;

    Item* nextFree;
    size_t count;

    static const size_t itemSize;

};

// ensures itemSize is at least 8 bytes
// needs to be able to store the pointer for free list and the bool to signify free
template<class T>
const size_t MemPool<T>::itemSize = sizeof(Item) < 8 ? 8 : sizeof(Item);