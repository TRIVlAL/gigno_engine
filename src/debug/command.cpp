#include "command.h"
#include "application.h"

namespace gigno {

    Command *Command::s_pCommands = nullptr;

    Command::Command(const char *name, CommandCallback_t callback, const char * help_string) : 
        m_Name{name}, m_Callback{callback}, m_HelpString{help_string} {
        pNext = Command::s_pCommands;
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

    CONSOLE_COMMAND_HELP(help, "Usage : 'help' [command] to goet help on a specific command of 'help' to get a list of all console commands.") {
        Console *console = Application::Singleton()->Debug()->GetConsole();
        Command *current = Command::s_pCommands;
        if(args.GetArgC() == 0) {
            console->LogInfo("Usage : 'help' [command] to get help on a specific command. List of all console commands :"); 
            while (current) {
                console->LogInfo(MESSAGE_NO_TIME_CODE_BIT, "  - %s", current->GetName());
                current = current->pNext;
            }
        } else {
            while(current) {
                if(strcmp(current->GetName(), args.GetArg(0)) == 0) {
                    const char * help_str;
                    if(*current->GetHelpString() == '\0') {
                        help_str = "- no help specified -";
                    } else {
                        help_str = current->GetHelpString();
                    }
                    console->LogInfo("Command '%s' : %s", current->GetName(), help_str);
                    return;
                }
                current = current->pNext;
            }
            console->LogInfo("Command '%s' does not exist.", args.GetArg(0));
        }

    }
}