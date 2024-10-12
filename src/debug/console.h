#ifndef CONSOLE_H
#define CONSOLE_H

#include "../core_macros.h"

namespace gigno {
    enum ConsoleMessageType_t {
        CONSOLE_MESSAGE_INFO = 0,
        CONSOLE_MESSAGE_WARN = 1,
        CONSOLE_MESSAGE_ERR = 2
    };

    const bool CONSOLE_TO_COUT = true;

    class Console {
    public:
        void Log(const char *msg, ConsoleMessageType_t type);
    private:
    };

}

#endif