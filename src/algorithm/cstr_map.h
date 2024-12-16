#ifndef CSTR_MAP_H
#define CSTR_MAP_H

#include <cstring>
#include <map>
#include <unordered_map>

namespace gigno {

    typedef const char *cstr;

    struct CstrSmaller_Func {
        bool operator()(const cstr & a, const cstr &b) const { 
            return strcmp(a, b) < 0;
        }
    };

    struct CstrEqual_Func {
        bool operator()(const cstr &a, const cstr &b) const {
            return strcmp(a, b) == 0;
        }
    };

    struct CstrHash_Func {
        size_t operator()(const cstr &from) const {
            int i = 0;
            size_t result = 0;
            while(from[i] != '\0') {
                result += from[i++];
            }
            return result;
        }
    };

    template<class TValue>
    using CstrMap_t = std::map<const char *, TValue, CstrSmaller_Func>;

    template<class TValue>
    using CstrUnorderedMap_t = std::unordered_map<const char *, TValue, CstrHash_Func, CstrEqual_Func>;

}

#endif
