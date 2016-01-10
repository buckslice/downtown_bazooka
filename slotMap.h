#pragma once
#include "entity.h"
#include <vector>

// shouldnt do this for entitys
// should do it for physicstransforms maybe?
//class SlotMap {
//public:
//
//    Entity* get_obj(long long id) {
//        Entity* e = table[(id & 0xFFFFFFFF) / chunk_size] + ((id & 0xFFFFFFFF) % chunk_size);
//        return e->id != id ? nullptr : e;
//    }
//
//    void destroy_obj(long long id) {
//        Entity* e = get_obj(id);
//        e->id = (e->id & 0xFFFFFFFF) | (((e->id >> 32) + 1) << 32);
//        free.push_back(id & 0xFFFFFFFF);
//    }
//    
//private:
//    const size_t chunk_size = 256;
//    std::vector<Entity*> table;
//    std::vector<int> free;
//};