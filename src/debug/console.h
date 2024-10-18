#ifndef CONSOLE_H
#define CONSOLE_H

#include "../core_macros.h"

#include <stdarg.h>
#include <vector>
#include <memory>
#include <ctime>
#include <fstream>
#include <filesystem>

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
        time_t TimePoint;
    private:
        const size_t Size;
    };

    const bool CONSOLE_TO_PRINTF = true;

    class Console {
        friend class DebugServer;
    public:
        ~Console();

        bool StartFileLogging();
        bool StopFileLogging();

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
        void DrawConsoleWindow(bool *open);

        void LogToFile(const ConsoleMessage_t &message);

    #if USE_IMGUI && USE_CONSOLE && USE_DEBUG_SERVER
        bool m_ShowTimepoints = true;
        std::vector<ConsoleMessage_t> m_Messages{};
        const std::filesystem::path m_Filepath{"log.txt"};
        std::ofstream m_FileStream;
        bool m_IsLoggingToFile = false;
        bool m_UIFileLoggingCheckbox;
        bool m_IsFirstFileOpen = true;
    #endif
    };

}

#endif