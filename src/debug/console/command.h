#ifndef COMMAND_H
#define COMMAND_H

#include "../../features_usage.h"

#if USE_CONSOLE

#include "command_token.h"

namespace gigno {

    class Command {
    public:
        typedef void(CommandCallback_t)(const CommandToken_t &);

        Command() = delete;
        Command(const char *name, CommandCallback_t callback, const char *help_string = "");

        Command* pNext{};

        const char *GetName() { return m_Name; }
        const char *GetHelpString() { return m_HelpString; }

        void Execute(const CommandToken_t &token);
        
        static Command* s_pCommands;
    private:
        CommandCallback_t *m_Callback;
        const char *m_Name;
        const char *m_HelpString;
    };
}
#endif

#if USE_CONSOLE
    #define CONSOLE_COMMAND(name)                \
    static void name(const CommandToken_t &args);\
    static Command Command_##name(#name, name);  \
    void name(const CommandToken_t &args) //{ }
#else
    #define CONSOLE_COMMAND(name) /* Console disabled : Console Commands are stripped from this build. Enable them in features_usage.h*/
#endif

#if USE_CONSOLE
    #define CONSOLE_COMMAND_HELP(name, help_string)   \
    static void name(const CommandToken_t &args);\
    static Command Command_##name(#name, name, help_string);  \
    void name(const CommandToken_t &args) //{ }
#else
#define CONSOLE_COMMAND_HELP(name, help_string) /* Console diabled : Console Commands are stripped from this build. Enable them in features_usage.h*/
#endif


#endif