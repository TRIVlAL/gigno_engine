#include "command.h"
#include "convar.h"
#include "../../application.h"

#include "../../features_usage.h"

#if USE_CONSOLE
namespace gigno {

    Command *Command::s_pCommands = nullptr;

    Command::Command(const char *name, CommandCallback_t callback, const char * help_string) : 
        m_Name{name}, m_Callback{callback}, m_HelpString{help_string} {
        m_pNext = Command::s_pCommands;
        Command::s_pCommands = this;
    }

    void Command::Execute(const CommandToken_t &token) {
        m_Callback(token);
    }

    //DEFINITIONS OF DEFAULT CONSOLE COMMANDS.

    CONSOLE_COMMAND_HELP(echo, "Usage : echo [words, ...].\nRepeats every arguments separated by a space.") {
        Console *console = Application::Singleton()->Debug()->GetConsole();
        ConsoleMessageFlags_t flags = MESSAGE_NO_NEW_LINE_BIT;
        for(int i = 0; i < args.GetArgC(); i++) {
            console->LogInfo(flags, "%s", args.GetArg(i));
            flags = (ConsoleMessageFlags_t)(MESSAGE_NO_NEW_LINE_BIT | MESSAGE_NO_TIME_CODE_BIT);
            if(i == args.GetArgC() - 2) {
                flags = MESSAGE_NO_TIME_CODE_BIT;
            }
        }
    }

    CONSOLE_COMMAND_HELP(cls, "Usage : clears the console from every messages.") {
        Console *console = Application::Singleton()->Debug()->GetConsole();
        console->m_Messages.clear();
    }

    CONSOLE_COMMAND_HELP(help, "Usage : 'help' [command] to get help on a specific command or 'help' to get a list of all console commands.") {
        Console *console = Application::Singleton()->Debug()->GetConsole();
        Command *comm_current = Command::s_pCommands;
        ConVar *convar_current = ConVar::s_pConVars;
        if(args.GetArgC() == 0) {
            console->LogInfo("Usage : 'help' [command] to get help on a specific command. List of all console commands :"); 
            if(comm_current) {
                console->LogInfo(MESSAGE_NO_TIME_CODE_BIT, "Commands :");
            }
            while (comm_current) {
                console->LogInfo(MESSAGE_NO_TIME_CODE_BIT, "  - %s", comm_current->GetName());
                comm_current = comm_current->GetNext();
            }
            if(convar_current) {
                console->LogInfo(MESSAGE_NO_TIME_CODE_BIT, "Console variables (ConVar) :");
            }
            while(convar_current) {
                console->LogInfo(MESSAGE_NO_TIME_CODE_BIT, "  - %s", convar_current->GetName());
                convar_current = convar_current->GetNext();
            }
        } else {
            while(comm_current) {
                if(strcmp(comm_current->GetName(), args.GetArg(0)) == 0) {
                    const char * help_str;
                    if(*comm_current->GetHelpString() == '\0') {
                        help_str = "- no help specified -";
                    } else {
                        help_str = comm_current->GetHelpString();
                    }
                    console->LogInfo("Command '%s' : %s", comm_current->GetName(), help_str);
                    return;
                }
                comm_current = comm_current->GetNext();
            }
            while(convar_current) {
                if(strcmp(convar_current->GetName(), args.GetArg(0)) == 0) {
                    console->LogInfo(MESSAGE_NO_NEW_LINE_BIT, "ConVar");
                    convar_current->PrintInfo(MESSAGE_NO_TIME_CODE_BIT | MESSAGE_NO_NEW_LINE_BIT);
                    console->LogInfo(MESSAGE_NO_TIME_CODE_BIT, "     %s", convar_current->GetHelpString());
                    return;
                }
                convar_current = convar_current->GetNext();
            }
            console->LogInfo("Command '%s' does not exist.", args.GetArg(0));
        }

    }
}

#endif //USE_CONSOLE