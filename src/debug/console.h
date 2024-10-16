#ifndef CONSOLE_H
#define CONSOLE_H

#include "../core_macros.h"

#include <stdarg.h>
#include <vector>
#include <memory>

namespace gigno {
    enum ConsoleMessageType_t {
        CONSOLE_MESSAGE_INFO = 0,
        CONSOLE_MESSAGE_WARN = 1,
        CONSOLE_MESSAGE_ERR = 2
    };

    struct ConsoleMessage_t {
        ConsoleMessage_t() = delete;
        ConsoleMessage_t(size_t size);
        ~ConsoleMessage_t();

        ConsoleMessageType_t Type;
        std::shared_ptr<char[]> Message;
    private:
        const size_t Size;
    };

    const bool CONSOLE_TO_COUT = true;

    class Console {
        std::vector<ConsoleMessage_t> m_Messages{};
    public:
        /*
        @brief logs a formatted info message to the console. 
        Follows the standard c formatting rules. 
        If the message cannot be formatted, a segmentation fault will occur (crash).
        */
        template<typename ...Args>
        void LogInfo(const char *fmt, Args ...args) {
            LogFormat(fmt, CONSOLE_MESSAGE_INFO, args...);
        }
        /*
        @brief logs a formatted warning message to the console.
        Follows the standard c formatting rules.
        If the message cannot be formatted, a segmentation fault will occur (crash).
        */
       template<typename ...Args>
        void LogWarning(const char *fmt, Args ...args) {
            LogFormat(fmt, CONSOLE_MESSAGE_WARN, args...);
        }
        /*
        @brief logs a formatted error message to the console.
        Follows the standard c formatting rules.
        If the message cannot be formatted, a segmentation fault will occur (crash).
        The error is just for look, it will not handle anything else.
        If you want correct error, use the ERR_MSG macro from error_macros.h
        */
       template<typename ...Args>
        void LogError(const char *fmt, Args ...args) {
            LogFormat(fmt, CONSOLE_MESSAGE_ERR, args...);
        }

        void LogInfo(const char *msg);
        void LogWarning(const char *msg);
        void LogError(const char *msg);
    private:
        void LogFormat(const char *fmt, ConsoleMessageType_t type, ...);
        void Log(const char *msg, ConsoleMessageType_t type);
    };

}

#endif