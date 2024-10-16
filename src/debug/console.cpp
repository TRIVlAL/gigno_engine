#include "console.h"
#include <iostream>

#include "../error_macros.h"

namespace gigno {

    ConsoleMessage_t::ConsoleMessage_t(size_t size) : Size{size}, Message{new char[size]} {
    }

    ConsoleMessage_t::~ConsoleMessage_t() {
    }

    void Console::LogInfo(const char *msg) {
        Log(msg, CONSOLE_MESSAGE_INFO);
    }

    void Console::LogWarning(const char *msg) {
        Log(msg, CONSOLE_MESSAGE_WARN);
    }

    void Console::LogError(const char *msg) {
        Log(msg, CONSOLE_MESSAGE_ERR);
    }

    void Console::Log(const char *msg, ConsoleMessageType_t type) {
        size_t msg_size = strlen(msg) + 1;
        ConsoleMessage_t &message = m_Messages.emplace_back(msg_size);
        memcpy(message.Message.get(), msg, msg_size);
        message.Type = type;

    #if USE_IMGUI
        if (CONSOLE_TO_COUT)
    #endif
        {
            std::cout << msg << std::endl;
        }
    }

    void Console::LogFormat(const char *fmt, ConsoleMessageType_t type, ...) {
        va_list params;
        va_list params2;
        va_start(params, type);
        va_copy(params2, params);

        size_t size;
        char *res_message;

        // Format size/is formattable
        size_t size_formatted;
        size_formatted = vsnprintf(nullptr, 0, fmt, params) + 1;

        //Create console message object.
        if(size_formatted < 0) {
            //failed to format string. Simply use unformated string.
            size = strlen(fmt) + 1;
            ConsoleMessage_t &message = m_Messages.emplace_back(size);
            strcpy(message.Message.get(), fmt);
            message.Type = type;
            res_message = message.Message.get();
        } else {
            size = size_formatted;
            ConsoleMessage_t &message = m_Messages.emplace_back(size);
            vsnprintf(message.Message.get(), size_formatted, fmt, params2);
            message.Type = type;
            res_message = message.Message.get();
        }


        // Also log to printf
    #if USE_IMGUI
        if(CONSOLE_TO_COUT) 
    #endif
        {
            std::cout  <<  res_message << std::endl;
        }
    }

}