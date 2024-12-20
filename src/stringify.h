
/*
    For a type to be considered fully-stringify-able, it must specify its type as a template of the 3 following methods :
        - size_t ToString<T>(char *to, const T& from)
        - std::pair<int, T> FromString<T>(const char **arguments, size_t argsCount);
        - constexpr const char *TypeString<T>();
*/

#ifndef STRINGIFY_H
#define STRINGIFY_H

#include <stdio.h>
#include <limits>
#include <utility>
#include <string>
#include <cstring>
#include <cerrno>
#include <string>
#include "debug/console/command_token.h"
#include "glm/glm.hpp"

namespace gigno {

// TO STRING -------------------------------------------

    /*
    ToString(char *to, const T& from)
        - Must return the number of characters the that resulting string will be, INCLUDING null-termaination character.
        - If 'to' is not nullptr, must write the resulting string to 'to'. MUST NOT PERFORM BOUNDS-CHECKING.
    */
    template<class T>
    size_t ToString(char *to, const T& from);

    /*
        Helper function that creates a std::string and fills it using the ToString(char *to, const T& from) function.
        You should always prioritize the ToString(char *to, const T& from) function, wich performs better !
    */
    template<typename T>
    std::string ToString(const T& from) {
        size_t size = ToString<T>(nullptr, from);
        std::string ret;
        ret.resize(size);
        ToString<T>(ret.data(), from);
        return ret;
    }

    template<> inline
    size_t ToString<int>(char *to, const int& from) {
        size_t size = snprintf(nullptr, 0, "%d", from) + 1;
        if(to) 
            snprintf(to, size, "%d", from);
        return size;
    }

    template<> inline
    size_t ToString<unsigned int>(char *to, const unsigned int& from) {
        size_t size = snprintf(nullptr, 0, "%u", from) + 1;
        if(to)
            snprintf(to, size, "%u", from);
        return size;
    }

    template<> inline
    size_t ToString<float>(char *to, const float& from) {
        size_t size = snprintf(nullptr, 0, "%f", from) + 1;
        if(to)
            snprintf(to, size, "%f", from);
        return size;
    }

    template<> inline
    size_t ToString<glm::vec3>(char *to, const glm::vec3 & from) {
        size_t size = snprintf(nullptr, 0, "(%f, %f, %f)", from.x, from.y, from.z) + 1;
        if(to)
            snprintf(to, size, "(%f, %f, %f)", from.x, from.y, from.z);
        return size;
    }

    template<> inline
    size_t ToString<bool>(char *to, const bool & from) {
        if(to) {
            if(from == true) {
                strcpy(to, "true");
            } else {
                strcpy(to, "false");
            }
        }

        if(from == true) {
            return 5; //true\0
        } else {
            return 6; //false\0
        }
    }

    typedef const char * cstr;

    template<> inline
    size_t ToString<cstr>(char *to, const cstr & from) {
        size_t size = strlen(from) + 1;
        if(to) {
            memcpy(to, from, size);
        }
        return size;
    }

// FROM STRING -----------------------------------------
    enum : int {
        FROM_STRING_SUCCESS = 0,

        FROM_STRING_NUMBER_OUT_OF_RANGE = 1,
        FROM_STRING_ONE_ARG_FAILED = 2, //Could not convert from the argument given (expects a single argument)

        FROM_STRING_ENUM_VALUE_NOT_EXISTS = 3,

        FROM_STRING_NOT_ENOUGH_ARGS = 4
    };

    /*
    FromString(const char **arguments, size_t argsCount)
        Converts a c-array of c-strings to a value.
        - Returns a pair : 
            1. int, 0 is success. 
            2. T, the resulting value, no guarantie for the value in case of failure. 
        - Some value may require more than one c-string as argument. arguments is a c-array of the various c-strings.
          argsCount is the length of this c-array.
    */
    template<class T>
    std::pair<int, T> FromString(const char **arguments, size_t argsCount);

    /*
    Helper function that uses converts a CommandToken_t to a 'const char**' and then calls FromString(const char **arguments, size_t argsCount).
    */
    template<typename T>
    std::pair<int, T> FromString(const CommandToken_t &args) {
        uint32_t argc = args.GetArgC();

        char* a[argc];
        for(int i = 0; i < argc; i++) {
            a[i] = new char[strlen(args.GetArg(i)) + 1];
            strcpy(a[i], args.GetArg(i));
        }

        auto result = FromString<T>((const char **)a, (size_t)argc);

        for(int i = 0; i < argc; i++) {
            delete[] a[i];
        }

        return result;
    }

    template<> inline
    std::pair<int, int> FromString<int>(const char **arguments, size_t argsCount) {
        char *endptr{};

        long val = strtol(arguments[0], &endptr, 10);

        int res = 0;
        if(endptr == arguments[0] || *endptr != '\0') { res = FROM_STRING_ONE_ARG_FAILED; }
        if(errno == ERANGE) {
            res = FROM_STRING_NUMBER_OUT_OF_RANGE; 
            errno = 0;
        }

        return std::pair<int, int>{res, val};
    }

    template<> inline
    std::pair<int, unsigned int> FromString<unsigned int>(const char **arguments, size_t argsCount)  {
        char *endptr{};

        unsigned int val = strtoul(arguments[0], &endptr, 10);

        int res = 0;
        if(endptr == arguments[0] || *endptr != '\0') { res = FROM_STRING_ONE_ARG_FAILED; }
        if(errno == ERANGE) { 
            res = FROM_STRING_NUMBER_OUT_OF_RANGE; 
            errno = 0;
        }

        return std::pair<int, unsigned int>{res, val};
    }

    template<> inline
    std::pair<int, float> FromString<float>(const char **arguments, size_t argsCount) {
        char *endptr{};

        float val = strtof(arguments[0], &endptr);

        int res = 0;
        if(endptr == arguments[0] || *endptr != '\0') { res = FROM_STRING_ONE_ARG_FAILED; }
        if(errno == ERANGE) { 
            res = FROM_STRING_NUMBER_OUT_OF_RANGE; 
            errno = 0;
        }

        return std::pair<int, float>{res, val};
    }

    template<> inline
    std::pair<int, glm::vec3> FromString<glm::vec3>(const char **arguments, size_t argsCount) {
        if(argsCount < 3) {
            return std::pair<int, glm::vec3>(FROM_STRING_NOT_ENOUGH_ARGS, glm::vec3{});
        }
        glm::vec3 ret{};

        std::pair<int, float> intermed_result = FromString<float>(&arguments[0], 1);
        if(intermed_result.first != FROM_STRING_SUCCESS) {
            return std::pair<int, glm::vec3>(intermed_result.first, glm::vec3{});
        } else {
            ret.x = intermed_result.second;
        }

        intermed_result = FromString<float>(&arguments[1], 1);
        if(intermed_result.first != FROM_STRING_SUCCESS) {
            return std::pair<int, glm::vec3>(intermed_result.first, glm::vec3{});
        } else {
            ret.y = intermed_result.second;
        }

        intermed_result = FromString<float>(&arguments[2], 1);
        if(intermed_result.first != FROM_STRING_SUCCESS) {
            return std::pair<int, glm::vec3>(intermed_result.first, glm::vec3{});
        } else {
            ret.z = intermed_result.second;
        }

        return std::pair<int, glm::vec3>(FROM_STRING_SUCCESS, ret);
    }

    template<> inline
    std::pair<int, bool> FromString<bool>(const char **arguments, size_t argsCount) {
        if (strcmp(*arguments, "true") == 0 || strcmp(*arguments, "1") == 0) {
            return std::pair<int, bool>{FROM_STRING_SUCCESS, true};
        } else if(strcmp(*arguments, "false") == 0 || strcmp(*arguments, "0") == 0) {
            return std::pair<int, bool>{FROM_STRING_SUCCESS, false};
        } else {
            return std::pair<int, bool>{FROM_STRING_ONE_ARG_FAILED, false};
        }
    }

    template<> inline
    std::pair<int, cstr> FromString<cstr>(const char **arguments, size_t argsCount) {
        size_t len = strlen(arguments[0]);
        char *ret = new char[len];
        strcpy(ret, arguments[0]);
        return std::pair<int, cstr>{FROM_STRING_SUCCESS, ret};
    }

//TYPE STRING -------------------------------------------------------------------------
    template<typename T>
    constexpr const char *TypeString();

    template<> inline
    constexpr const char *TypeString<int>() { return "int"; }

    template<> inline
    constexpr const char *TypeString<unsigned int>() { return "unsigned int"; }
    
    template<> inline
    constexpr const char *TypeString<float>() { return "float"; }

    template<> inline
    constexpr const char *TypeString<glm::vec3>() { return "vec3"; }

    template<> inline
    constexpr const char *TypeString<bool>() { return "bool"; }

    template<> inline
    constexpr const char *TypeString<const char *>() { return "const char *"; }

}

#endif