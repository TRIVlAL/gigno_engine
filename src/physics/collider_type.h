#ifndef COLLIDER_TYPE_H
#define COLLIDER_TYPE_H

#include <utility>
#include "../stringify.h"

namespace gigno {

    enum ColliderType_t {
        COLLIDER_NONE = 0,
        COLLIDER_HULL = 1, //Must be the first enum entry.
        COLLIDER_SPHERE = 2,
        COLLIDER_PLANE = 3,
        COLLIDER_CAPSULE = 4,
        COLLIDER_MAX_ENUM = 5
    };

    template<> inline
    size_t ToString<ColliderType_t>(char *to, const ColliderType_t &from) {
        switch(from) {
            case COLLIDER_NONE : 
                if(to) {
                    strcpy(to, "COLLIDER_NONE");
                }
                return 14;
            case COLLIDER_SPHERE:
                if (to) {
                    strcpy(to, "COLLIDER_SPHERE");
                }
                return 16;
            case COLLIDER_PLANE:
                if (to) {
                    strcpy(to, "COLLIDER_PLANE");
                }
                return 15;
            case COLLIDER_CAPSULE:
                if (to) {
                    strcpy(to, "COLLIDER_CAPSULE");
                }
                return 17;
            case COLLIDER_HULL:
                if(to) {
                    strcpy(to, "COLLIDER_HULL");
                }
                return 14;
            default:
                if(to) {
                    strcpy(to, "?");
                }
                return 2;
        }
    }

    template<> inline
    std::pair<int, ColliderType_t> FromString(const char **arguments, size_t argsCount) {
        if(strcmp(arguments[0], "COLLIDER_NONE") == 0 || strcmp(arguments[0], "0") == 0) {
            return std::pair<int, ColliderType_t>{FROM_STRING_SUCCESS, COLLIDER_NONE};
        } else if(strcmp(arguments[0], "COLLIDER_HULL") == 0 || strcmp(arguments[0], "1") == 0) {
            return std::pair<int, ColliderType_t>{FROM_STRING_SUCCESS, COLLIDER_HULL};
        } else if(strcmp(arguments[0], "COLLIDER_SPHERE") == 0 || strcmp(arguments[0], "2") == 0) {
            return std::pair<int, ColliderType_t>{FROM_STRING_SUCCESS, COLLIDER_SPHERE};
        } else if(strcmp(arguments[0], "COLLIDER_PLANE") == 0 || strcmp(arguments[0], "3") == 0){
            return std::pair<int, ColliderType_t>{FROM_STRING_SUCCESS, COLLIDER_PLANE};
        } else if(strcmp(arguments[0], "COLLIDER_CAPSULE") == 0 || strcmp(arguments[0], "4") == 0) {
            return std::pair<int, ColliderType_t>{FROM_STRING_SUCCESS, COLLIDER_CAPSULE};
        } else {
            return std::pair<int, ColliderType_t>{FROM_STRING_ONE_ARG_FAILED, COLLIDER_NONE};
        }
    }
}

#endif