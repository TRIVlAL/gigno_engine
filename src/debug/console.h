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
        CONSOLE_MESSAGE_ERR = 2,
        CONSOLE_MESSAGE_ECHO = 3
    };

    enum ConsoleMessageFlags_t {
        MESSAGE_NO_NEW_LINE_BIT = 1 << 0,
        MESSAGE_NO_TIME_CODE_BIT = 1 << 1,
        MESSAGE_NO_FILE_LOG_BIT = 1 << 2
    };

    struct ConsoleMessage_t {
        ConsoleMessage_t() = delete;
        ConsoleMessage_t(size_t size);
        ~ConsoleMessage_t();

        ConsoleMessageType_t Type;
        ConsoleMessageFlags_t Flags;
        std::shared_ptr<char> Message;
        time_t TimePoint;
    private:
        const size_t Size;
    };

    const bool CONSOLE_TO_PRINTF = false;
    const int CONSOLE_MAX_MESSAGE_TO_RENDER = 12'000;     // 0 for rendering all messages.
    const int CONSOLE_RENDER_FIRST_MESSAGE_COUNT = 2'000; // If there are more messages than the CONSOLE_MAX_MESSAGE_TO_RENDER,
                                                          // CONSOLE_RENDER_FIRST_MESSAGE_COUNT of the first messages will be rendered,
                                                          // and the rest will be filled out by the most recent messages.

    struct CommandToken_t;
    static void cls(const CommandToken_t&); // Defined in command.cpp.
                                            // forward-declared here so it can be made a friend of Console.

    class Console {
        friend class DebugServer;

        friend void cls(const CommandToken_t&);
    public:
        Console();
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
            LogFormat(fmt, CONSOLE_MESSAGE_INFO, (ConsoleMessageFlags_t)0, args...);
        }
        template <typename... Args>
        void LogInfo(ConsoleMessageFlags_t flags, const char *fmt, Args... args) {
            LogFormat(fmt, CONSOLE_MESSAGE_INFO, flags, args...);
        }
        /*
        @brief logs a formatted warning message to the console.
        Follows the standard c formatting rules.
        If the message cannot be formatted, a segmentation fault will occur (crash).
        */
       template<typename ...Args>
        void LogWarning(const char *fmt, Args ...args) {
           LogFormat(fmt, CONSOLE_MESSAGE_WARN, (ConsoleMessageFlags_t)0, args...);
        }
        template <typename... Args>
        void LogWarning(ConsoleMessageFlags_t flags, const char *fmt, Args... args) {
            LogFormat(fmt, CONSOLE_MESSAGE_WARN, flags, args...);
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
           LogFormat(fmt, CONSOLE_MESSAGE_ERR, (ConsoleMessageFlags_t)0, args...);
        }
        template <typename... Args>
        void LogError(ConsoleMessageFlags_t flags, const char *fmt, Args... args) {
            LogFormat(fmt, CONSOLE_MESSAGE_ERR, flags, args...);
        }

        void LogInfo(const char *msg);
        void LogWarning(const char *msg);
        void LogError(const char *msg);

        void CallCommand(const char *line);
    private:
        void LogFormat(const char *fmt, ConsoleMessageType_t type, ConsoleMessageFlags_t flags, ...);
        void Log(const char *msg, ConsoleMessageType_t type, ConsoleMessageFlags_t flags);
        void DrawConsoleTab();

        void LogToFile(const ConsoleMessage_t &message);

    #if USE_IMGUI && USE_CONSOLE && USE_DEBUG_SERVER
        bool m_ShowTimepoints = true;
        std::vector<ConsoleMessage_t> m_Messages{};
        const std::filesystem::path m_Filepath{"log.txt"};
        std::ofstream m_FileStream;
        bool m_IsLoggingToFile = false;
        bool m_UIFileLoggingCheckbox;
        bool m_IsFirstFileOpen = true;

        const static size_t CONSOLE_INPUT_BUFFER_SIZE = 256;
        char m_InputBuffer[CONSOLE_INPUT_BUFFER_SIZE];
    #endif
    };

}

#endif