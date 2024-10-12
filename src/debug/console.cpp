#include "console.h"
#include <iostream>

namespace gigno {

    void Console::Log(const char *msg, ConsoleMessageType_t type) {
    #if USE_IMGUI
        if(CONSOLE_TO_COUT) {
            std::cout << "THIS IS THE CONSOLE TALKING : " <<  msg << std::endl;
        }
    #else
        std::cout << begin << msg << end << std::endl;
    #endif
    }

}