#include "console.h"
#include <iostream>
#include <chrono>

#include "../error_macros.h"

namespace gigno {

    ConsoleMessage_t::ConsoleMessage_t(size_t size) : Size{size}, Message{new char[size]} {
    }

    ConsoleMessage_t::~ConsoleMessage_t() {
    }

    Console::~Console() {
    #if USE_CONSOLE && USE_IMGUI && USE_DEBUG_SERVER
        if(m_IsLoggingToFile) {
            StopFileLogging();
        }
    #endif
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
    #if USE_CONSOLE && USE_IMGUI && USE_DEBUG_SERVER
        size_t msg_size = strlen(msg) + 1;
        ConsoleMessage_t &message = m_Messages.emplace_back(msg_size);
        memcpy(message.Message.get(), msg, msg_size);
        message.Type = type;
        message.TimePoint = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        LogToFile(message);


        if (CONSOLE_TO_PRINTF)
    #endif
        {
            printf(msg);
            printf("\n");
        }
    }

    void Console::LogFormat(const char *fmt, ConsoleMessageType_t type, ...) {
    #if USE_CONSOLE && USE_IMGUI && USE_DEBUG_SERVER
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
            message.TimePoint = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            LogToFile(message);
        } else {
            size = size_formatted;
            ConsoleMessage_t &message = m_Messages.emplace_back(size);
            vsnprintf(message.Message.get(), size_formatted, fmt, params2);
            message.Type = type;
            res_message = message.Message.get();
            message.TimePoint = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
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
        va_start(params, type);
        vprintf(fmt, params);
        printf("\n");
    #endif
    }

    bool Console::StartFileLogging() {
        #if USE_IMGUI && USE_CONSOLE && USE_DEBUG_SERVER
        if(m_IsLoggingToFile) {
            return true;
        }
        if (!m_FileStream.is_open()) {
            if(m_IsFirstFileOpen) {
                m_FileStream.open(m_Filepath);
                m_IsFirstFileOpen = false;
            } else {
                m_FileStream.open(m_Filepath, std::ios::app);
            }
            if(!m_FileStream.is_open()) {
                Console::LogError("Failed to open file %s for logging !", m_Filepath.c_str());
                return false;
            } else {
                m_IsLoggingToFile = true;
            }
        }
        m_UIFileLoggingCheckbox = true;
        LogInfo("Started logging to file.");
        return true;
        #endif
    }

    bool Console::StopFileLogging() {
        #if USE_IMGUI && USE_CONSOLE && USE_DEBUG_SERVER
        if(!m_IsLoggingToFile) {
            return true;
        }
        if(m_FileStream.is_open()) {
            m_FileStream << "Logging ended. Closing.\n";
            m_FileStream.close();
            m_IsLoggingToFile = false;
        }
        LogInfo("Stoppend logging to file.");
        m_UIFileLoggingCheckbox = false;
        return true;
        #endif
    }

    void Console::LogToFile(const ConsoleMessage_t &message) {
        #if USE_IMGUI && USE_CONSOLE && USE_DEBUG_SERVER
        if(m_IsLoggingToFile) {
            m_FileStream << "[" << message.TimePoint/3600%24 << ":" << message.TimePoint/60%60 << ":" << message.TimePoint%60 << "] " << message.Message.get() << "\n";
        }
        #endif
    }

    #if USE_IMGUI
    void Console::DrawConsoleWindow(bool *open) {
        #if USE_IMGUI && USE_CONSOLE && USE_DEBUG_SERVER
        if(!*open) {
            return;
        }

        if(!ImGui::Begin("Console", open)) {
            ImGui::End();
            return;
        }

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
        ImGui::Text("(%s)", m_Filepath.u8string().c_str());

        ImGui::Separator();

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4{0.2f, 0.2f, 0.2f, 0.8f});
        if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), ImGuiChildFlags_NavFlattened/*, ImGuiWindowFlags_HorizontalScrollbar*/)) {
            for(ConsoleMessage_t &message : m_Messages) {
                // Set color.
                ImVec4 color{1.0f, 1.0f, 1.0f, 1.0f};
                if(message.Type == CONSOLE_MESSAGE_WARN) {
                    color = ImVec4{0.8f, 0.8f, 0.0f, 1.0f};
                } else if(message.Type == CONSOLE_MESSAGE_ERR) {
                    color = ImVec4{1.0f, 0.0f, 0.0f, 1.0f};
                }

                ImGui::PushStyleColor(ImGuiCol_Text, color);
                if(m_ShowTimepoints) {
                    int hours = message.TimePoint/3600%24;
                    int min = message.TimePoint/60%60;
                    int sec = message.TimePoint%60;
                    ImGui::Text("[%d:%d:%d] ", hours, min, sec);
                    ImGui::SameLine();
                }
                ImGui::TextWrapped(message.Message.get());
                ImGui::PopStyleColor();
            }
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY();
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::End();
        #endif
    }
    #endif

}