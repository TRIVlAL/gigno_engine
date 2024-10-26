#ifndef CONSOLE_H
#define CONSOLE_H

#include "../../features_usage.h"

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

    const bool CONSOLE_TO_PRINTF = true;                  // Are Console messages also forwarded to the standard console output ? (printf)
    const int CONSOLE_RENDER_FIRST_MESSAGE_COUNT = 2'000; // If there are more messages than the convar_console_max_message.Get(),
                                                          // CONSOLE_RENDER_FIRST_MESSAGE_COUNT of the first messages will be rendered,
                                                          // and the rest will be filled out by the most recent messages.
    const std::filesystem::path CONSOLE_LOG_FILEPATH{"log.txt"}; // The relative path to the file where the messages are logged
                                                                 // When StartFileLogging() is called.

    struct CommandToken_t;
    static void cls(const CommandToken_t&); // Defined in command.cpp. Method of the cls console command to clear the console.
                                            // forward-declared here so it can be made a friend of Console.

    class Console {
        friend class DebugServer;

        friend void cls(const CommandToken_t&);
    public:
        Console();
        ~Console();

        /*
        @brief From now on, until a StopFileLogging() call, every messages will be copied into a log file.
        This Log file is defined as the constant CONSOLE_LOG_FILEPATH (see above).
        @returns whether opening the file was successfull.
        Notes : - If the file does not exist, it is created.
                - If the file already exists and it is the first time Logging to file since startup, all the content
                of the file is OVEWRITEN
                - The content of the log file will not necessarly appear externaly until a StopFileLogging call or
                  the termination of the application.
        */
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

        /*
        No-formatting versions of the log methods.
        */
        void LogInfo(const char *msg);
        void LogWarning(const char *msg);
        void LogError(const char *msg);

    private:
        void CallCommand(const char *line);

        void LogFormat(const char *fmt, ConsoleMessageType_t type, ConsoleMessageFlags_t flags, ...);
        void Log(const char *msg, ConsoleMessageType_t type, ConsoleMessageFlags_t flags);
        void DrawConsoleTab();

        void LogToFile(const ConsoleMessage_t &message);

    #if USE_CONSOLE
        bool m_ShowTimepoints = true;
        std::vector<ConsoleMessage_t> m_Messages{};
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