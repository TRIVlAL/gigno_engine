#ifndef ERROR_H
#define ERROR_H

#include "application.h"

#include <cstdio>

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)

#define ERR_MSG(msg, ...)                                                               \
    Console::LogError("ERROR " __FILE__ " line " LINE_STRING " : " msg, ##__VA_ARGS__); \
    return

#define ERR_MSG_V(ret, msg, ...) \
ERR_MSG(msg , ##__VA_ARGS__) ret

#define ERR                                                        \
    Console::LogError("ERROR " __FILE__ " line " LINE_STRING "."); \
    return

#define ERR_V(ret) \
    ERR ret

#define ERR_NULL_MSG(val, msg, ...)  \
    if (val == NULL)                 \
    {                                \
        ERR_MSG(msg, ##__VA_ARGS__); \
    }

#define ERR_NULL(val) \
    if (val == NULL)  \
    {                 \
        ERR           \
    }

#define ASSERT_MSG(cond, msg, ...)                                                                     \
    if (!(cond))                                                                                       \
    {                                                                                                  \
        Console::LogError("ASSERT FAILED at " __FILE__ " line " LINE_STRING " : " msg, ##__VA_ARGS__); \
        __debugbreak();                                                                                \
        return;                                                                                        \
    }

#define ASSERT_MSG_V(cond, ret, msg, ...)                                                                \
    if (!(cond))                                                                                         \
    {                                                                                                    \
        Console::LogError("ASSERT FAILED at " __FILE__ " line " LINE_STRING " : " msg, ##__VA_ARGS__); \
        __debugbreak();                                                                                  \
        return ret;                                                                                      \
    }

#define ASSERT_V(cond, ret)                                                       \
    if (!(cond))                                                                  \
    {                                                                             \
        Console::LogError("ASSERT FAILED at " __FILE__ " line " LINE_STRING "."); \
        __debugbreak();                                                           \
        return ret;                                                               \
    }

#define ASSERT(cond)                                                               \
    if (!(cond))                                                                   \
    {                                                                              \
             Console::LogError ("ASSERT FAILED at " __FILE__ " line " LINE_STRING "."); \
        __debugbreak();                                                            \
        return;                                                                    \
    }

#endif