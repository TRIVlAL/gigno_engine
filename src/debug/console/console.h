#ifndef CONSOLE_H
#define CONSOLE_H

#include "../../features_usage.h"

#include <stdarg.h>
#include <vector>
#include <memory>
#include <ctime>
#include <fstream>
#include <filesystem>
#include <mutex>

struct ImGuiInputTextCallbackData;

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

        ConsoleMessageType_t Type{};
        ConsoleMessageFlags_t Flags{};
        std::shared_ptr<char> Message{};
        time_t TimePoint{};
    private:
        const size_t Size{};
    };

    const bool CONSOLE_TO_PRINTF = true;                  // Are Console messages also forwarded to the standard console output ? (printf)
    const int CONSOLE_RENDER_FIRST_MESSAGE_COUNT = 2'000; // If there are more messages than the convar_console_max_message,
                                                          // CONSOLE_RENDER_FIRST_MESSAGE_COUNT of the first messages will be rendered,
                                                          // and the rest will be filled out by the most recent messages.
    const std::filesystem::path CONSOLE_LOG_FILEPATH{"gigno_log.txt"}; // The relative path to the file where the messages are logged
                                                                    // When StartFileLogging_Impl() is called.

    const size_t CONSOLE_COMMAND_MAX_ARG_COUNT = 10;
    const size_t CONSOLE_COMMAND_MAX_LENGTH = 255;      //The max number of character in a console command call.

    struct CommandToken_t {
        char Data[CONSOLE_COMMAND_MAX_LENGTH];
        char *Name;
        char *Args[CONSOLE_COMMAND_MAX_ARG_COUNT];
        size_t ArgCount;
    };

    static void cls(const CommandToken_t&); // Defined in command.cpp. Method of the cls console command to clear the console.
                                            // forward-declared here so it can be made a friend of Console.
    static void bind_update(float);         // Defined in bind_command. forward-declared here so it can be made a friend of Console.
    static void phys_remote_update(float) ; // Defined in phys_remote_command.h

    class Console {
        friend class DebugServer;
        friend class EntityServer;
        friend void cls(const CommandToken_t&);
        friend void bind_update(float);
    public:
        // Static methods forwarded to singleton.

        /*
        @brief From now on, until a StopFileLogging() call, every messages will be copied into a log file.
        This Log file is defined as the constant CONSOLE_LOG_FILEPATH (see above).
        @returns whether opening the file was successfull.
        Notes : - If the file does not exist, it is created.
                - If the file already exists and it is the first time Logging to file since startup, all the content
                of the file is OVEWRITEN
                - The content of the log file will not necessarly appear externaly until a StopFileLoggin call or
                  the termination of the application.
        */
        static bool StartFileLogging() { return Singleton()->StartFileLogging_Impl(); }
        static bool StopFileLogging() { return Singleton()->StopFileLogging_Impl(); }

        static void LogInfo(const char *message) { Singleton()->LogInfo_Impl(message); }
        template <typename... Args>
        /*
        @brief logs a formatted info message to the console. 
        Follows the standard c formatting rules. 
        If the message cannot be formatted, a segmentation fault will occur (crash).
        */
        static void LogInfo(const char *formatted, Args... args) { Singleton()->LogInfo_Impl(formatted, args...); }
        template<typename ...Args> 
        static void LogInfo(ConsoleMessageFlags_t flags, const char *formatted, Args ...args) { Singleton()->LogInfo_Impl(flags, formatted, args...); }

        static void LogWarning(const char *message) { Singleton()->LogWarning_Impl(message); }
        template<typename ...Args> 
        /*
        @brief logs a formatted warning message to the console.
        Follows the standard c formatting rules.
        If the message cannot be formatted, a segmentation fault will occur (crash).
        */
        static void LogWarning(const char *formatted, Args ...args) { Singleton()->LogWarning_Impl(formatted, args...); }
        template <typename... Args>
        static void LogWarning(ConsoleMessageFlags_t flags, const char *formatted, Args... args) { Singleton()->LogWarning_Impl(flags, formatted, args...); }

        static void LogError(const char *message) { Singleton()->LogError_Impl(message); }
        /*
        @brief logs a formatted error message to the console.
        Follows the standard c formatting rules.
        If the message cannot be formatted, a segmentation fault will occur (crash).
        The error is just for look, it will not handle anything else.
        If you want correct error, use the ERR_MSG macro from error_macros.h
        */
        template<typename ...Args> 
        static void LogError(const char *formatted, Args ...args) { Singleton()->LogError_Impl(formatted, args...); }
        template <typename... Args>
        static void LogError(ConsoleMessageFlags_t flags, const char *formatted, Args... args) { Singleton()->LogError_Impl(flags, formatted, args...); }

        static void CallCommand(const char *line) {
            return Singleton()->CallCommand_Impl(line);
        }

        static void CallCommandTokenized(const CommandToken_t &tokens) {
            return Singleton()->CallCommandTokenized_Impl(tokens);
        }

        static void ExecuteConfigFiles();

    private:
        static Console s_Instance;

        static Console *Singleton() {
            return &s_Instance;
        }

        Console();
        ~Console();

        
        bool StartFileLogging_Impl();
        bool StopFileLogging_Impl();

        
        template<typename ...Args>
        void LogInfo_Impl(const char *fmt, Args ...args) {
            LogFormat(fmt, CONSOLE_MESSAGE_INFO, (ConsoleMessageFlags_t)0, args...);
        }
        template <typename... Args>
        void LogInfo_Impl(ConsoleMessageFlags_t flags, const char *fmt, Args... args) {
            LogFormat(fmt, CONSOLE_MESSAGE_INFO, flags, args...);
        }
        
       template<typename ...Args>
        void LogWarning_Impl(const char *fmt, Args ...args) {
           LogFormat(fmt, CONSOLE_MESSAGE_WARN, (ConsoleMessageFlags_t)0, args...);
        }
        template <typename... Args>
        void LogWarning_Impl(ConsoleMessageFlags_t flags, const char *fmt, Args... args) {
            LogFormat(fmt, CONSOLE_MESSAGE_WARN, flags, args...);
        }
        
       template<typename ...Args>
        void LogError_Impl(const char *fmt, Args ...args) {
           LogFormat(fmt, CONSOLE_MESSAGE_ERR, (ConsoleMessageFlags_t)0, args...);
        }
        template <typename... Args>
        void LogError_Impl(ConsoleMessageFlags_t flags, const char *fmt, Args... args) {
            LogFormat(fmt, CONSOLE_MESSAGE_ERR, flags, args...);
        }

        /*
        Called by EntityServer in Think... better way?
        */
        void UpdateCommands(float dt);

        /*
        No-formatting versions of the log methods.
        */
        void LogInfo_Impl(const char *msg);
        void LogWarning_Impl(const char *msg);
        void LogError_Impl(const char *msg);

        void CallCommand_Impl(const char *line);
        void CallCommandTokenized_Impl(const CommandToken_t &tokens);

        void LogFormat(const char *fmt, ConsoleMessageType_t type, ConsoleMessageFlags_t flags, ...);
        void Log(const char *msg, ConsoleMessageType_t type, ConsoleMessageFlags_t flags);
        void DrawConsoleTab();

        /*
        Writes the message to file if it does not have the flag MESSAGE_NO_FILE_LOG_BIT
        Flushes the fileif proority is MESSAGE_TYPE_WARNING or higher.
        */
        void LogToFile(const ConsoleMessage_t &message);

        
        std::mutex m_LogMutex;
        std::mutex m_MessageVectorMutex;
        #if USE_CONSOLE
        
        bool m_ShowTimepoints = true;
        std::vector<ConsoleMessage_t> m_Messages{};
        std::ofstream m_FileStream;
        bool m_IsLoggingToFile = false;
        bool m_UIFileLoggingCheckbox;
        bool m_IsFirstFileOpen = true;
        
        /*
        Input field for the commands.
        */
        char m_InputBuffer[CONSOLE_COMMAND_MAX_LENGTH];
        
        void Clear();
        void ExecuteConfigFile(std::filesystem::directory_entry file);

        bool m_AutoCompleteInitialized = false;
        void InitializeAutocomplete();                      //to be called after every convar/commands have been registered
        void UpdateAutocomplete(char *input);                          //sorts the OrderedAutoocompleteResult depending on the input.
        static int InputEditCallback(ImGuiInputTextCallbackData* data);

        size_t m_AutocompleteResultCount = 0;               // number of autocomplete result
        std::vector<const char *> m_OrderedAutocompleteResult;    // vector containing every command/convar names, by order of autocomplete validity.
        size_t m_AutocompleteFocusIndex = 0;                //0 means not in focus. 1 means 1st choice, etc...
        #endif
    };

    /*
    Sets up the signal and termination handlers, if they haven't yuet been defined.
    */
    void InitializeErrorHandling();
    //Termination Handler
    void OnTerminate();
    // Signal Handler
    void OnSignal(int signal);

}

#endif