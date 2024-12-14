#ifndef COMMAND_H
#define COMMAND_H

#include "../../features_usage.h"

#if USE_CONSOLE

#include "command_token.h"

namespace gigno {

    class Command {
    public:
        typedef void(CommandCallback_t)(const CommandToken_t &);
        typedef void(CommandUpdate_t)(float);

        Command() = delete;
        Command(const char *name, CommandCallback_t callback, CommandUpdate_t update_callback, const char *help_string = "");

        Command *GetNext() { return m_pNext; }

        const char *GetName() { return m_Name; }
        const char *GetHelpString() { return m_HelpString; }

        void Execute(const CommandToken_t &token);

        void Update(float dt);
        
        static Command* s_pCommands;
    private:
        Command* m_pNext{};

        CommandCallback_t *m_Callback;
        CommandUpdate_t *m_Update;
        const char *m_Name;
        const char *m_HelpString;
    };
}
#endif

#if USE_CONSOLE
    #define CONSOLE_COMMAND(name)                \
    static void name(const CommandToken_t &args);\
    static void name##_update(float dt){}\
    static Command Command_##name(#name, name, name##_update);  \
    void name(const CommandToken_t &args) //{ }
#else
    #define CONSOLE_COMMAND(name) /* Console disabled : Console Commands are stripped from this build. Enable them in features_usage.h*/
#endif

#if USE_CONSOLE
    #define CONSOLE_COMMAND_HELP(name, help_string)   \
    static void name(const CommandToken_t &args);\
    static void name##_update(float dt){}\
    static Command Command_##name(#name, name, name##_update, help_string);  \
    void name(const CommandToken_t &args) //{ }
#else
#define CONSOLE_COMMAND_HELP(name, help_string) /* Console diabled : Console Commands are stripped from this build. Enable them in features_usage.h*/
#endif

#if USE_CONSOLE
    #define CONSOLE_COMMAND_HELP_UPDATE(name, help_string)   \
    static void name(const CommandToken_t &args);\
    static void name##_update(float dt); /* Defined by user.*/\
    static Command Command_##name(#name, name, name##_update, help_string);  \
    void name(const CommandToken_t &args) //{ }
#else
#define CONSOLE_COMMAND_HELP_UPDATE(name, help_string) /* Console diabled : Console Commands are stripped from this build. Enable them in features_usage.h*/
#endif

#endif