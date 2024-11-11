#include "profiler.h"
#include "cstring"
#include "../../rendering/gui.h"
#include <thread>

namespace gigno {

    #if USE_PROFILER
    std::mutex Profiler::s_BindThreadMutex{};
    std::vector<Profiler::ProfileThread*> Profiler::s_BoundThreads{};

    thread_local Profiler::ProfileThread Profiler::s_Thread{};
    #endif

    void Profiler::Begin(const char *name) {
        #if USE_PROFILER
        s_Thread.Begin(name);
        #endif
    }

    void Profiler::End() {
        #if USE_PROFILER
        s_Thread.End();
        #endif
    }

    void Profiler::DrawProfilerTab() {
        #if USE_PROFILER
        s_BindThreadMutex.lock();
        for(ProfileThread *thread : s_BoundThreads) {
            thread->DrawUI();
        }
        s_BindThreadMutex.unlock();
        #endif
    }

    #if USE_PROFILER
    Profiler::ProfileThread::ProfileThread() {
        m_ThreadHash = (int)std::hash<std::thread::id>()(std::this_thread::get_id());
        static int i = 0;
        m_ThreadID = i++;
        s_BindThreadMutex.lock();
        s_BoundThreads.push_back(this);
        s_BindThreadMutex.unlock();
    }

    Profiler::ProfileThread::~ProfileThread() {
        s_BindThreadMutex.lock();
        for(size_t i = 0; i < s_BoundThreads.size(); i++) {
            if(s_BoundThreads[i] == this) {
                s_BoundThreads.erase(s_BoundThreads.begin() + i);
                break;
            }
        }
        s_BindThreadMutex.unlock();
    }

    void Profiler::ProfileThread::Begin(const char *name)
    {
        ProfileScope_t *active_scope = &m_RootScope;

        if(!m_StartedRootScope) {
            //The root scope never started. It will be our first scope.
            m_StartedRootScope = true;
            m_RootScope.Name = name;
            m_RootScope.Start();
            return;
        } else {
            while(active_scope->ActiveChildIndex != -1) {
                active_scope = active_scope->Children[active_scope->ActiveChildIndex];
            }
        }


        for(int i = 0; i < active_scope->Children.size(); i++) {
            if(strcmp(name, active_scope->Children[i]->Name) == 0) {
                // scope with this name already exists.
                active_scope->Children[i]->Start();
                active_scope->ActiveChildIndex = i;
                return;
            }
        }

        //No scope with the same name is child. Create one.
        ProfileScope_t *child = active_scope->Children.emplace_back(new ProfileScope_t{name});

        active_scope->ActiveChildIndex = active_scope->Children.size() - 1;

        child->Start();
    }

    void Profiler::ProfileThread::End() {
        if(m_RootScope.ActiveChildIndex == -1) {
            // The root scope is closed. End of frame.
            m_RootScope.Stop();
            EndFrame();
            return;
        }

        ProfileScope_t *active_scope = &m_RootScope;
        ProfileScope_t *last_scope = active_scope;

        while(active_scope->ActiveChildIndex != -1) {
            last_scope = active_scope;
            active_scope = active_scope->Children[active_scope->ActiveChildIndex];
        }

        active_scope->Stop();
        last_scope->ActiveChildIndex = -1;
    }

    Profiler::ProfileThread::ProfileScope_t::~ProfileScope_t() {
        // TODO : SEGFAULT ON DESTRUCTOR !

        // These deletes cause crashes fsr... 
        // We risk to leack them, but that poses no problem since we only delete ProfileScopes at app exit.

        /*
        for(ProfileScope_t* child : Children) {
            delete child;
        }
        */
    }

    void Profiler::ProfileThread::ProfileScope_t::Start()
    {
        m_StartTime = std::chrono::high_resolution_clock::now();
        HasRun = true;
    }

    void Profiler::ProfileThread::ProfileScope_t::Stop() {
        std::chrono::nanoseconds nano_dur = std::chrono::high_resolution_clock::now() - m_StartTime;
        float micro_dur = nano_dur.count() * PROFILER_FROM_NANOSECOND_CONVERTION;

        m_TotalDurationThisFrame += micro_dur;
        m_CallCountThisFrame++;
    }

    void Profiler::ProfileThread::EndFrame() {
        m_RootScope.EndFrame();
        m_StartedRootScope = false;
    }

    void Profiler::ProfileThread::ProfileScope_t::EndFrame() {
        // Update the data
        Data.CallCountThisFrame = m_CallCountThisFrame;
        m_CallCountThisFrame = 0;

        Data.RecentTotal -= Data.Durations[Data.CurrentIndex];
        Data.Durations[Data.CurrentIndex] = m_TotalDurationThisFrame;
        Data.RecentTotal += m_TotalDurationThisFrame;

        Data.CurrentIndex++;
        if (Data.CurrentIndex >= PROFILER_RESOLUTION)
        {
            Data.CurrentIndex = 0;
        }

        if (Data.MaxTotalDuration < m_TotalDurationThisFrame)
        {
            Data.MaxTotalDuration = m_TotalDurationThisFrame;
        }

        Data.UpdateCeilling(m_TotalDurationThisFrame);

        m_TotalDurationThisFrame = 0.0f;

        //Call EndFrame to children
        for(ProfileScope_t *child : Children) {
            child->EndFrame();
        }
    }

    void Profiler::ProfileThread::ProfileScope_t::ProfileData_t::UpdateCeilling(float micro_duration) {
        if(micro_duration > CurrentCeilling) {
            float total = AverageValueOverCeilling * ValuesOverCeillingCount;
            total += micro_duration;
            ValuesOverCeillingCount++;
            FramesWithoutValuesOverCeilling = 0;
            AverageValueOverCeilling = total / ValuesOverCeillingCount;
        } else {
            FramesWithoutValuesOverCeilling++;
        }
        if(ValuesOverCeillingCount > 5) {
            CurrentCeilling = AverageValueOverCeilling;
            ValuesOverCeillingCount = 0;
            AverageValueOverCeilling = 0.0f;
        }
        if(RecentTotal / PROFILER_RESOLUTION < CurrentCeilling * 0.35f) {
            CurrentCeilling -= (ValuesOverCeillingCount > 0? .1f : CurrentCeilling * 0.001f + 3.0f);
        }
        if(FramesWithoutValuesOverCeilling > 4000) {
            ValuesOverCeillingCount = 0;
        }
    }

    void Profiler::ProfileThread::DrawUI() {
        m_RootScope.DrawUI(0, m_ThreadID, m_ThreadHash);
    }

    void Profiler::ProfileThread::ProfileScope_t::DrawUI(int depth, int thread_id, int thread_hash) {
        
        //Copy data
        DataCopy = Data;

        size_t label_size{};
        if(depth == 0) {
            label_size = snprintf(nullptr, 0, "%s on thread %d", Name, thread_id) + 1;
        } else {
            label_size = strlen(Name) + 1;
        }
        char label[label_size];
        if(depth == 0) {
            snprintf(label, label_size, "%s on thread %d", Name, thread_id);
        } else {
            strcpy(label, Name);
        }

        if(ImGui::TreeNode(label)) {

            float width = 500 - (float)depth * ImGui::GetTreeNodeToLabelSpacing();
            float height = std::max(100.0f - (float)depth * 20.0f, 40.0f);
            ImGui::PlotLines("##Durations", DataCopy.Durations, PROFILER_RESOLUTION, DataCopy.CurrentIndex, nullptr, 0.0f,
                             DataCopy.CurrentCeilling + 1.0f, ImVec2{width, height});
            ImGui::SameLine();
            float posX = ImGui::GetCursorPosX();
            ImGui::Text("%d us", (int)DataCopy.CurrentCeilling);

            ImGui::SameLine();
            ImGui::SetCursorPosX(posX);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + height/2 -5.0f);
            ImGui::Text("%d us", (int)DataCopy.Durations[DataCopy.CurrentIndex]);

            ImGui::SameLine();
            ImGui::SetCursorPosX( posX);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() +  height - 10.0f);
            ImGui::Text("%d us", (int)0.0f);

            ImGui::Text("Recent Average :");
            ImGui::SameLine(); 
            ImGui::SetCursorPosX(width/2);
            int avrg = (int)(DataCopy.RecentTotal / PROFILER_RESOLUTION);
            ImGui::Text("%d us", avrg);

            ImGui::Text("Max Value :");
            ImGui::SameLine();
            ImGui::SetCursorPosX(width/2);
            ImGui::Text("%d us", (int)DataCopy.MaxTotalDuration);

            if(DataCopy.CallCountThisFrame == 1) {
                ImGui::Text("Called once.");
            } else {
                ImGui::Text("Called %d times this frame", DataCopy.CallCountThisFrame);
                ImGui::Text("Average per call :");
                ImGui::SameLine(); 
                ImGui::SetCursorPosX(width/2);
                int avrg = (int)(DataCopy.Durations[DataCopy.CurrentIndex] / DataCopy.CallCountThisFrame);
                ImGui::Text("%d us", avrg);

                /*ImGui::Text("Max Duration :");
                ImGui::SameLine(); 
                ImGui::SetCursorPosX(width/2);
                ImGui::Text("%d us", (int)DataCopy.MaxTotalDuration);*/
            }

            if(Children.size() > 0)
            {
                ImGui::SeparatorText("Contains :");
            }
            for(ProfileScope_t *child : Children) {
                child->DrawUI(depth + 1, thread_id, thread_hash);
            }
            ImGui::TreePop();
        }
    }

    #endif // USE_PROFILER


}