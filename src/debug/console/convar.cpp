#include "convar.h"
#include "application.h"

namespace gigno {

    #if USE_CONSOLE
    
    void BaseConvar::LogInfo() const {
        char valstr[ValToString(nullptr)];
        ValToString(valstr);
        Application::Singleton()->Debug()->GetConsole()->LogInfo("'%s' = '%s'", GetName(), valstr);
    }

    void BaseConvar::HandleSetResult(const CommandToken_t &args, int result) const {
        Console *console = Application::Singleton()->Debug()->GetConsole();
        switch(result) {
            case FROM_STRING_SUCCESS:
                LogInfo(); break;
            case FROM_STRING_ONE_ARG_FAILED:
                console->LogInfo("Could not trivially convert '%s' to type '%s'", args.GetArg(0), TypeToString()); break;
            case FROM_STRING_NUMBER_OUT_OF_RANGE:
                console->LogInfo("Value '%s' was too big/small. It exceeded the limit for the type '%s'", args.GetArg(0), TypeToString()); break;
            default:
                console->LogInfo("Could not set Convar !"); break;
        }
    }

    #endif

}