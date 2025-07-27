#include "command.h"
#include "convar.h"
#include "../../application.h"

#include "../../features_usage.h"

#include <assert.h>

#if USE_CONSOLE
namespace gigno {

    Command *Command::s_pCommands = nullptr;

    Command::Command(const char *name, CommandCallback_t callback, CommandUpdate_t update_callback, const char * help_string) : 
        m_HelpString{help_string}, m_Name{name}, m_Update{update_callback}, m_Callback{callback} {
        m_pNext = Command::s_pCommands;
        Command::s_pCommands = this;
    }

    void Command::Execute(const CommandToken_t &token) {
        m_Callback(token);
    }

    void Command::Update(float dt) {
        m_Update(dt);
    }

    //DEFINITIONS OF BASIC CONSOLE COMMANDS.

    CONSOLE_COMMAND_HELP(echo, "Usage : echo [words, ...].\nRepeats every arguments separated by a space.") {

        ConsoleMessageFlags_t flags = MESSAGE_NO_NEW_LINE_BIT;

        for(int i = 0; i < COMMAND_ARG_COUNT; i++) {

            COMMAND_ARG_I_STR(i, argument);

            Console::LogInfo (flags, "%s", argument);

            flags = (ConsoleMessageFlags_t)(MESSAGE_NO_NEW_LINE_BIT | MESSAGE_NO_TIME_CODE_BIT);
            if(i == COMMAND_ARG_COUNT - 1) {
                flags = MESSAGE_NO_TIME_CODE_BIT;
            }

        }

    }

    CONSOLE_COMMAND_HELP(cls, "Usage : clears the console from every messages.") {
        if(Console *console = Console::Singleton()) {
            console->Clear();
        }
    }

    CONSOLE_COMMAND_HELP(help, "Usage : 'help' [command] to get help on a specific command or 'help' to get a list of all console commands.") {

        Command *comm_current = Command::s_pCommands;
        BaseConvar *convar_current = BaseConvar::s_pConvars;

        if(COMMAND_ARG_COUNT == 0) {
            Console::LogInfo ("Usage : 'help' [command] to get help on a specific command. List of all console commands :"); 
            if(comm_current) {
                Console::LogInfo (MESSAGE_NO_TIME_CODE_BIT, "Commands :");
            }
            while (comm_current) {
                Console::LogInfo (MESSAGE_NO_TIME_CODE_BIT, "  - %s", comm_current->GetName());
                comm_current = comm_current->GetNext();
            }
            if(convar_current) {
                Console::LogInfo (MESSAGE_NO_TIME_CODE_BIT, "Console variables (ConVar) :");
            }
            while(convar_current) {
                Console::LogInfo (MESSAGE_NO_TIME_CODE_BIT, "  - %s", convar_current->GetName());
                convar_current = convar_current->GetNext();
            }

            return;
        }
        
        COMMAND_REQUIRE_EQU_ARG_COUNT(1);

        COMMAND_ARG_0_STR(comm_name);

        while(comm_current) {
            if(strcmp(comm_current->GetName(), comm_name) == 0) {
                const char * help_str;
                if(*comm_current->GetHelpString() == '\0') {
                    help_str = "- no help specified -";
                } else {
                    help_str = comm_current->GetHelpString();
                }
                Console::LogInfo ("Command '%s' : %s", comm_name, help_str);
                return;
            }

            comm_current = comm_current->GetNext();

        }
        while(convar_current) {
            if(strcmp(convar_current->GetName(), comm_name) == 0) {
                char valuestr[convar_current->ValToString(nullptr)];
                convar_current->ValToString(valuestr);
                Console::LogInfo ("Convar (%s) '%s' = '%s' : %s", convar_current->TypeToString(), convar_current->GetName(), valuestr, convar_current->GetHelpString());
                return;
            }
            convar_current = convar_current->GetNext();
        }

        Console::LogInfo ("Command '%s' does not exist.", comm_name);
    }

    CONSOLE_COMMAND_HELP(exit, "closes the app") {

        if(Application *app = Application::Singleton()) {
            app->SetExit(EXIT_SIMPLE);
        }

    }

    CONSOLE_COMMAND_HELP(status, "prints infos to the console") {
        if(Application *app = Application::Singleton()) {
            Console::LogInfo("Current map : '%s'"
            , app->m_CurrentMap.c_str());
        }
    }
}

#endif //USE_CONSOLE