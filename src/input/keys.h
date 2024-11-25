#ifndef KEYS_H
#define KEYS_H

#include "../stringify.h"

namespace gigno {

    //Keys are taken from the GLFW standard.

    #define KEY_LIST\
    Key(HAT_CENTERED, 0)\
    Key(HAT_UP, 1)\
    Key(HAT_RIGHT, 2)\
    Key(HAT_DOWN, 4)\
    Key(HAT_LEFT, 8)\
    Key(HAT_RIGHT_UP, (HAT_RIGHT | HAT_UP))\
    Key(HAT_RIGHT_DOWN, (HAT_RIGHT | HAT_DOWN))\
    Key(HAT_LEFT_UP, (HAT_LEFT | HAT_UP))\
    Key(HAT_LEFT_DOWN, (HAT_LEFT | HAT_DOWN))\
    Key(KEY_UNKNOWN, -1)\
    Key(KEY_SPACE, 32)\
    Key(KEY_APOSTROPHE, 39)\
    Key(KEY_COMMA, 44)\
    Key(KEY_MINUS, 45)\
    Key(KEY_PERIOD, 46)\
    Key(KEY_SLASH, 47)\
    Key(KEY_0, 48)\
    Key(KEY_1, 49)\
    Key(KEY_2, 50)\
    Key(KEY_3, 51)\
    Key(KEY_4, 52)\
    Key(KEY_5, 53)\
    Key(KEY_6, 54)\
    Key(KEY_7, 55)\
    Key(KEY_8, 56)\
    Key(KEY_9, 57)\
    Key(KEY_SEMICOLON, 59)\
    Key(KEY_EQUAL, 61)\
    Key(KEY_A, 65)\
    Key(KEY_B, 66)\
    Key(KEY_C, 67)\
    Key(KEY_D, 68)\
    Key(KEY_E, 69)\
    Key(KEY_F, 70)\
    Key(KEY_G, 71)\
    Key(KEY_H, 72)\
    Key(KEY_I, 73)\
    Key(KEY_J, 74)\
    Key(KEY_K, 75)\
    Key(KEY_L, 76)\
    Key(KEY_M, 77)\
    Key(KEY_N, 78)\
    Key(KEY_O, 79)\
    Key(KEY_P, 80)\
    Key(KEY_Q, 81)\
    Key(KEY_R, 82)\
    Key(KEY_S, 83)\
    Key(KEY_T, 84)\
    Key(KEY_U, 85)\
    Key(KEY_V, 86)\
    Key(KEY_W, 87)\
    Key(KEY_X, 88)\
    Key(KEY_Y, 89)\
    Key(KEY_Z, 90)\
    Key(KEY_LEFT_BRACKET, 91)\
    Key(KEY_BACKSLASH, 92)\
    Key(KEY_RIGHT_BRACKET, 93)\
    Key(KEY_GRAVE_ACCENT, 96)\
    Key(KEY_WORLD_1, 161)\
    Key(KEY_WORLD_2, 162)\
    Key(KEY_ESCAPE, 256)\
    Key(KEY_ENTER, 257)\
    Key(KEY_TAB, 258)\
    Key(KEY_BACKSPACE, 259)\
    Key(KEY_INSERT, 260)\
    Key(KEY_DELETE, 261)\
    Key(KEY_RIGHT, 262)\
    Key(KEY_LEFT, 263)\
    Key(KEY_DOWN, 264)\
    Key(KEY_UP, 265)\
    Key(KEY_PAGE_UP, 266)\
    Key(KEY_PAGE_DOWN, 267)\
    Key(KEY_HOME, 268)\
    Key(KEY_END, 269)\
    Key(KEY_CAPS_LOCK, 280)\
    Key(KEY_SCROLL_LOCK, 281)\
    Key(KEY_NUM_LOCK, 282)\
    Key(KEY_PRINT_SCREEN, 283)\
    Key(KEY_PAUSE, 284)\
    Key(KEY_F1, 290)\
    Key(KEY_F2, 291)\
    Key(KEY_F3, 292)\
    Key(KEY_F4, 293)\
    Key(KEY_F5, 294)\
    Key(KEY_F6, 295)\
    Key(KEY_F7, 296)\
    Key(KEY_F8, 297)\
    Key(KEY_F9, 298)\
    Key(KEY_F10, 299)\
    Key(KEY_F11, 300)\
    Key(KEY_F12, 301)\
    Key(KEY_F13, 302)\
    Key(KEY_F14, 303)\
    Key(KEY_F15, 304)\
    Key(KEY_F16, 305)\
    Key(KEY_F17, 306)\
    Key(KEY_F18, 307)\
    Key(KEY_F19, 308)\
    Key(KEY_F20, 309)\
    Key(KEY_F21, 310)\
    Key(KEY_F22, 311)\
    Key(KEY_F23, 312)\
    Key(KEY_F24, 313)\
    Key(KEY_F25, 314)\
    Key(KEY_KP_0, 320)\
    Key(KEY_KP_1, 321)\
    Key(KEY_KP_2, 322)\
    Key(KEY_KP_3, 323)\
    Key(KEY_KP_4, 324)\
    Key(KEY_KP_5, 325)\
    Key(KEY_KP_6, 326)\
    Key(KEY_KP_7, 327)\
    Key(KEY_KP_8, 328)\
    Key(KEY_KP_9, 329)\
    Key(KEY_KP_DECIMAL, 330)\
    Key(KEY_KP_DIVIDE, 331)\
    Key(KEY_KP_MULTIPLY, 332)\
    Key(KEY_KP_SUBTRACT, 333)\
    Key(KEY_KP_ADD, 334)\
    Key(KEY_KP_ENTER, 335)\
    Key(KEY_KP_EQUAL, 336)\
    Key(KEY_LEFT_SHIFT, 340)\
    Key(KEY_LEFT_CONTROL, 341)\
    Key(KEY_LEFT_ALT, 342)\
    Key(KEY_LEFT_SUPER, 343)\
    Key(KEY_RIGHT_SHIFT, 344)\
    Key(KEY_RIGHT_CONTROL, 345)\
    Key(KEY_RIGHT_ALT, 346)\
    Key(KEY_RIGHT_SUPER, 347)\
    Key(KEY_MENU, 348)\
    Key(KEY_MAX_ENUM, 349)

    #define Key(key, val) key = val,

    enum Key_t {
        KEY_LIST // Define enum using macros so they can be reused when defining ToString and FromString
    };

    #undef Key

    #define Key(key, val) case key:\
        if(to) {\
            strcpy(to, #key);\
        }\
        return 20;\

    template<> inline
    size_t ToString<Key_t>(char *to, const Key_t & from) {
        switch(from) {
            KEY_LIST
            default:
                return 20;
        }
    }

    #undef Key

    #define Key(key, val) else if (strcmp(arguments[0], #key) == 0) {\
        return std::pair<int, Key_t>{FROM_STRING_SUCCESS, key}; }
    
    template<> inline
    std::pair<int, Key_t> FromString<Key_t>(const char **arguments, size_t argsCount)  {
        if(false) {;}
        KEY_LIST

        //else
        return std::pair<int, Key_t>{FROM_STRING_ENUM_VALUE_NOT_EXISTS, KEY_MAX_ENUM};
    }

    #undef Key
    #undef KEY_LIST

    template <> 
    inline constexpr const char *TypeString<Key_t>() { return "Key_t"; }
}

#endif