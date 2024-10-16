#ifndef ERROR_H
#define ERROR_H

#include "application.h"

#include <cstdio>

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)

#define ERR_MSG(msg, ...)                                                                        \
    if (Application *app = Application::Singleton())                                             \
    {                                                                                            \
        app->Debug()->GetConsole()->LogError("ERROR " __FILE__ " line " LINE_STRING " : " msg, ##__VA_ARGS__); \
    }                                                                                            \
    else                                                                                         \
    {                                                                                            \
        printf("ERROR " __FILE__ " line " LINE_STRING " : " msg "\n", ##__VA_ARGS__);            \
    }                                                                                            \
    return

#define ERR_MSG_V(ret, msg, ...) \
ERR_MSG(msg , ##__VA_ARGS__) ret

#define ERR                                                                 \
    if (Application *app = Application::Singleton())                        \
    {                                                                       \
        app->Debug()->GetConsole()->LogError("ERROR " __FILE__ " line " LINE_STRING "."); \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        printf("ERROR " __FILE__ " line " LINE_STRING ".\n");               \
    }                                                                       \
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

#define ASSERT_MSG(cond, msg, ...)                                                                                                   \
    if (!(cond))                                                                                                                     \
    {                                                                                                                                \
        if (Application *app = Application::Singleton())                                                                             \
        {                                                                                                                            \
            Application::Singleton()->Debug()->GetConsole()->LogError("ASSERT FAILED at " __FILE__ " line " LINE_STRING " : " msg, ##__VA_ARGS__); \
        }                                                                                                                            \
        else                                                                                                                         \
        {                                                                                                                            \
            printf("ASSERT FAILED at " __FILE__ " line " LINE_STRING " : " msg "\n", ##__VA_ARGS__);                                 \
        }                                                                                                                            \
                                                                                                                                     \
        __debugbreak();                                                                                                              \
        return;                                                                                                                      \
    }

#define ASSERT_MSG_V(cond, ret, msg, ...)                                                                                            \
    if (!(cond))                                                                                                                     \
    {                                                                                                                                \
        if (Application *app = Application::Singleton())                                                                             \
        {                                                                                                                            \
            Application::Singleton()->Debug()->GetConsole()->LogError("ASSERT FAILED at " __FILE__ " line " LINE_STRING " : " msg, ##__VA_ARGS__); \
        }                                                                                                                            \
        else                                                                                                                         \
        {                                                                                                                            \
            printf("ASSERT FAILED at " __FILE__ " line " LINE_STRING " : " msg "\n", ##__VA_ARGS__);                                 \
        }                                                                                                                            \
                                                                                                                                     \
        __debugbreak();                                                                                                              \
        return ret;                                                                                                                  \
    }

#define ASSERT_V(cond, ret)                                                                                     \
    if (!(cond))                                                                                                \
    {                                                                                                           \
        if (Application *app = Application::Singleton())                                                        \
        {                                                                                                       \
            Application::Singleton()->Debug()->GetConsole()->LogError("ASSERT FAILED at " __FILE__ " line " LINE_STRING "."); \
        }                                                                                                       \
        else                                                                                                    \
        {                                                                                                       \
            printf("ASSERT FAILED at " __FILE__ " line " LINE_STRING ".\n");                                    \
        }                                                                                                       \
                                                                                                                \
        __debugbreak();                                                                                         \
        return ret;                                                                                             \
    }

#define ASSERT(cond)                                                                                            \
    if (!(cond))                                                                                                \
    {                                                                                                           \
        if (Application *app = Application::Singleton())                                                        \
        {                                                                                                       \
            Application::Singleton()->Debug()->GetConsole()->LogError("ASSERT FAILED at " __FILE__ " line " LINE_STRING "."); \
        }                                                                                                       \
        else                                                                                                    \
        {                                                                                                       \
            printf("ASSERT FAILED at " __FILE__ " line " LINE_STRING ".\n");                                    \
        }                                                                                                       \
                                                                                                                \
        __debugbreak();                                                                                         \
        return;                                                                                                 \
    }

#endif