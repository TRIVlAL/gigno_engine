#ifndef ERROR_H
#define ERROR_H

#include "application.h"

#include <cstdio>

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)

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
    
    #define VULKAN_CHECK(function, error_message)VkResult result = function;                   \
    if (result != VK_SUCCESS)                                                              \
    {                                                                                      \
        Console::LogError("VULKAN " error_message " Vulkan Error Code : %d", (int)result); \
        Application::Singleton()->SetExit(EXIT_FAILED_RENDERER);                           \
    }
    
#endif