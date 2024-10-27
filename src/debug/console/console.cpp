#include "console.h"
#include <iostream>
#include <chrono>

#include "../../error_macros.h"
#include "command.h"
#include "convar.h"

namespace gigno {

    Convar<uint32_t> convar_console_max_message = Convar<uint32_t>("console_max_message", "Max number of messages rendered to the console. 0 = all messages", 15'000);

    ConsoleMessage_t::ConsoleMessage_t(size_t size) : Size{size}, Message{new char[size], std::default_delete<char[]>()} {
    }

    ConsoleMessage_t::~ConsoleMessage_t() {
    }

    Console::Console() {
        LogInfo((ConsoleMessageFlags_t)(MESSAGE_NO_TIME_CODE_BIT | MESSAGE_NO_FILE_LOG_BIT), "---------------------------------");
        LogInfo((ConsoleMessageFlags_t)(MESSAGE_NO_TIME_CODE_BIT | MESSAGE_NO_FILE_LOG_BIT), "Gigno engine console initialized.");
        LogInfo((ConsoleMessageFlags_t)(MESSAGE_NO_TIME_CODE_BIT | MESSAGE_NO_FILE_LOG_BIT), "---------------------------------");
        LogInfo((ConsoleMessageFlags_t)(MESSAGE_NO_TIME_CODE_BIT | MESSAGE_NO_FILE_LOG_BIT), "");
    }

    Console::~Console() {
    #if USE_CONSOLE
        if(m_IsLoggingToFile) {
            StopFileLogging();
        }
    #endif
    }

    void Console::LogInfo(const char *msg) {
        Log(msg, CONSOLE_MESSAGE_INFO, (ConsoleMessageFlags_t)0);
    }

    void Console::LogWarning(const char *msg) {
        Log(msg, CONSOLE_MESSAGE_WARN, (ConsoleMessageFlags_t)0);
    }

    void Console::LogError(const char *msg) {
        Log(msg, CONSOLE_MESSAGE_ERR, (ConsoleMessageFlags_t)0);
    }

    void Console::Log(const char *msg, ConsoleMessageType_t type, ConsoleMessageFlags_t flags) {
    #if USE_CONSOLE
        size_t msg_size = strlen(msg) + 1;
        ConsoleMessage_t &message = m_Messages.emplace_back(msg_size);
        memcpy(message.Message.get(), msg, msg_size);
        message.Type = type;
        message.TimePoint = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        message.Flags = flags;
        LogToFile(message);


        if (CONSOLE_TO_PRINTF)
    #endif
        {
            printf(msg);
            printf("\n");
        }
    }

    void Console::LogFormat(const char *fmt, ConsoleMessageType_t type, ConsoleMessageFlags_t flags, ...) {
    #if USE_CONSOLE
        va_list params;
        va_list params2;
        va_start(params, flags);
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
            message.TimePoint = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            message.Flags = flags;
            LogToFile(message);
        } else {
            size = size_formatted;
            ConsoleMessage_t &message = m_Messages.emplace_back(size);
            vsnprintf(message.Message.get(), size_formatted, fmt, params2);
            message.Type = type;
            res_message = message.Message.get();
            message.TimePoint = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            message.Flags = flags;
            LogToFile(message);
        }


        // Also log to printf
        if(CONSOLE_TO_PRINTF) 
        {
            printf(res_message);
            printf("\n");
        }
    #else
        va_list params;
        va_start(params, flags);
        vprintf(fmt, params);
        printf("\n");
    #endif
    }

    bool Console::StartFileLogging() {
        #if USE_CONSOLE
        if(m_IsLoggingToFile) {
            return true;
        }
        if (!m_FileStream.is_open()) {
            if(m_IsFirstFileOpen) {
                m_FileStream.open(CONSOLE_LOG_FILEPATH);
                m_IsFirstFileOpen = false;
            } else {
                m_FileStream.open(CONSOLE_LOG_FILEPATH, std::ios::app);
            }
            if(!m_FileStream.is_open()) {
                Console::LogError("Failed to open file %s for logging !", CONSOLE_LOG_FILEPATH.c_str());
                return false;
            } else {
                m_IsLoggingToFile = true;
            }
        }
        m_UIFileLoggingCheckbox = true;
        LogInfo("Started logging to file.");
        return true;
        #else
        return false;
        #endif
    }

    bool Console::StopFileLogging() {
        #if USE_CONSOLE
        if(!m_IsLoggingToFile) {
            return true;
        }
        if(m_FileStream.is_open()) {
            m_FileStream << "\nLogging ended. Closing.\n";
            m_FileStream.close();
            m_IsLoggingToFile = false;
        }
        LogInfo("Stopped logging to file.");
        m_UIFileLoggingCheckbox = false;
        return true;
        #else
        return false;
        #endif
    }

    void Console::LogToFile(const ConsoleMessage_t &message) {
        #if USE_CONSOLE
        if(m_IsLoggingToFile && !(message.Flags & MESSAGE_NO_FILE_LOG_BIT)) {
            if(!(message.Flags & MESSAGE_NO_TIME_CODE_BIT)) {
                m_FileStream << "[" << message.TimePoint/3600%24 << ":" << message.TimePoint/60%60 << ":" << message.TimePoint%60 << "] ";
            } 
            m_FileStream << message.Message.get();
            if(!(message.Flags & MESSAGE_NO_NEW_LINE_BIT)) {
                m_FileStream << "\n";
            }
        }
        #endif
    }

    void Console::CallCommand(const char *line) {
        #if USE_CONSOLE
        CommandToken_t token{line};
        if(!token.GetName()) {
            LogInfo("Invalid command call.");
            return;
        }

        {
            Command *current = Command::s_pCommands;
            while(current) {
                if(token.CompareName(current->GetName())) {
                    current->Execute(token);
                    return;
                }
                current = current->GetNext();
            }
        }
        {
            BaseConvar *current = BaseConvar::s_pConvars;
            while(current) {
                if(token.CompareName(current->GetName())) {
                    current->Set(token);
                    return;
                }
                current = current->GetNext();
            }
        }
        
        LogInfo("No Command/Convar '%s' found. use 'help' for the list of commands.", token.GetName());
        #endif
    }

    #if USE_IMGUI
    void Console::DrawConsoleTab() {
        #if USE_CONSOLE

        if(ImGui::Button("Clear")) { m_Messages.clear(); }
        ImGui::SameLine();
        ImGui::Checkbox("Show times", &m_ShowTimepoints);
        ImGui::SameLine();
        if(ImGui::Checkbox("Log to file", &m_UIFileLoggingCheckbox)) {
            if(m_UIFileLoggingCheckbox) {
                if(!StartFileLogging()) {
                    m_UIFileLoggingCheckbox = false;
                }
            } else {
                StopFileLogging();
            }
        }
        ImGui::SameLine();
        ImGui::Text("(%s)", CONSOLE_LOG_FILEPATH.u8string().c_str());

        ImGui::Separator();

        int render_first_message_count = convar_console_max_message > CONSOLE_RENDER_FIRST_MESSAGE_COUNT ? CONSOLE_RENDER_FIRST_MESSAGE_COUNT : 0;

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4{0.2f, 0.2f, 0.2f, 0.8f});
        if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -30.0f), ImGuiChildFlags_NavFlattened/*, ImGuiWindowFlags_HorizontalScrollbar*/)) {
            if(convar_console_max_message != 0 && m_Messages.size() > convar_console_max_message) {
                ImGui::TextWrapped("%d Messages. Messages rendered limited to %d (the first %d and last %d) ",
                                   m_Messages.size(),
                                   convar_console_max_message,
                                   render_first_message_count,
                                   convar_console_max_message - render_first_message_count);
                ImGui::Separator();
            }
            int i = 0;
            bool has_skipped = false;
            if(m_Messages.size() > convar_console_max_message && convar_console_max_message != 0 && render_first_message_count == 0) {
                i = m_Messages.size() - convar_console_max_message;
                has_skipped = true;
                ImGui::TextWrapped("---- Skipped %d messages ----", m_Messages.size() - convar_console_max_message);
            }
            while(i < m_Messages.size()) {
                ConsoleMessage_t& message = m_Messages[i];

                // Set color.
                ImVec4 color{1.0f, 1.0f, 1.0f, 1.0f};
                if(message.Type == CONSOLE_MESSAGE_WARN) {
                    color = ImVec4{0.8f, 0.8f, 0.0f, 1.0f};
                } else if(message.Type == CONSOLE_MESSAGE_ERR) {
                    color = ImVec4{1.0f, 0.0f, 0.0f, 1.0f};
                } else if(message.Type == CONSOLE_MESSAGE_ECHO) {
                    color = ImVec4{0.6f, 0.6f, 0.6f, 1.0f};
                }

                ImGui::PushStyleColor(ImGuiCol_Text, color);
                if(m_ShowTimepoints && !(message.Flags & MESSAGE_NO_TIME_CODE_BIT)) {
                    int hours = message.TimePoint/3600%24;
                    int min = message.TimePoint/60%60;
                    int sec = message.TimePoint%60;
                    ImGui::Text("[%d:%d:%d] ", hours, min, sec);
                    ImGui::SameLine();
                }
        
                ImGui::TextWrapped(message.Message.get());

                if(message.Flags & MESSAGE_NO_NEW_LINE_BIT) {
                    if(m_Messages.size() - 1 != i) {
                        ImGui::SameLine();
                    }
                }

                ImGui::PopStyleColor();

                i++;
                if(!has_skipped && m_Messages.size() > convar_console_max_message && convar_console_max_message != 0 && i >= render_first_message_count) {
                    ImGui::TextWrapped("---- Skipped %d messages ----", m_Messages.size() - convar_console_max_message);
                    i = m_Messages.size() - convar_console_max_message + render_first_message_count;
                    has_skipped = true;
                }
            }
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY();
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::Separator();
        if (ImGui::InputText("Enter command", m_InputBuffer, CONSOLE_INPUT_BUFFER_SIZE, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll)) {
            LogFormat(" -> %s", CONSOLE_MESSAGE_ECHO, (ConsoleMessageFlags_t)0, m_InputBuffer);
            CallCommand(m_InputBuffer);
            m_InputBuffer[0] = '\0';
            ImGui::SetKeyboardFocusHere(-1);
        }
        #endif
    }
    #endif


}