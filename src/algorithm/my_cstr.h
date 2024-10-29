#ifndef MY_CSTR_H
#define MY_CSTR_H

#include <cstring>

namespace gigno {

    /*
    replaced every instance of search by replace in the given string.
    returns wheter any was found.
    */
    bool str_replace_char(char *str, char search, char replace) {
        char *next{};
        next = strchr(str, search);
        bool got = false;
        while(next) {
            *next = replace;
            next = strchr(next, search);
            got = true;
        }
        return got;
    }

    void str_replace_inquote(char *str, char search, char replace, char quote = '"') {
        char *curr = str;
        bool in_quote = false;
        while(*curr != '\0') {
            if(*curr == quote) {
                in_quote = !in_quote;
            }

            if(in_quote && *curr == search) {
                *curr = replace;
            }

            curr ++;
        }
    }

}

#endif