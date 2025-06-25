#include "arena.h"
#include "../error_macros.h"
#include "../debug/console/console.h"

namespace gigno {

    Arena::Arena(size_t capactity) {
        ASSERT(capactity > 0);
        m_Capactity = capactity;
        m_pData = (char*)malloc(capactity);
        m_Bounds = {{USAGE_USED, 0},{USAGE_FREE, 0}};
        m_pNext = nullptr;
    }

    Arena::~Arena() {
        if(m_pNext) {
            delete m_pNext;
        }

        free(m_pData);
    }

    void *Arena::Alloc(size_t size) {
        ASSERT_V(size <= m_Capactity, nullptr);

        int i = m_Bounds.size();
        while(--i >= 0) {
            if(m_Bounds[i].Usage == USAGE_FREE) {
                size_t avaliable_space{};

                if(i == m_Bounds.size() - 1) {
                    avaliable_space = m_Capactity - m_Bounds[i].Position;
                } else {
                    avaliable_space = m_Bounds[i + 1].Position - m_Bounds[i].Position;
                }

                if(avaliable_space >= size)  {
                    void *ret = &m_pData[m_Bounds[i].Position];

                    m_Bounds[i].Position += size;
                    if(i != m_Bounds.size() - 1 && m_Bounds[i].Position == m_Bounds[i+1].Position) {
                        m_Bounds.erase(m_Bounds.begin() + i, m_Bounds.begin() + i + 1);
                    }

                    if(i == 0) {
                        m_Bounds[i].Position = 0;
                    }

                    return ret; 
                } else {
                    continue;
                }
            }
        }

        //no avaliable space !
        if(!m_pNext) {
            Console::LogWarning("Warning : Arena of '%d' bytes ran out of space and duplicated!"
                                " You may want to consider upping the default capacity!", m_Capactity);
            m_pNext = new Arena{m_Capactity};
        }
        return m_pNext->Alloc(size);
    }

    void Arena::Free(void *position, size_t size) {
        if((char*)position > m_pData && (char *)position < m_pData + m_Capactity) {
            // position is inside this arena.
            size_t index_position = (char*)position - m_pData;
            Free(index_position, size);
        } else {
            //position is in another side arena
            if(m_pNext) {
                return m_pNext->Free(position, size);
            }

            ASSERT(false) //There is no side arena ! position was never allocated in an arena.
        }
    }

    void Arena::Free(size_t position, size_t size) {
        size_t inside_bound_index{};

        //binary search of the bound with highest position smaller or equal to position.
        size_t bottom = 0;
        size_t top = m_Bounds.size() - 1;
        size_t current = top / 2;
        while(current != bottom) {
            if(m_Bounds[current].Position > position) {
                top = current;
                current = (bottom + top)/2; 
            } else if(m_Bounds[current].Position < position) {
                bottom = current;
                current = (bottom + top)/2;
            } else {
                inside_bound_index = current;
                break;
            }
        }
        
        ASSERT(m_Bounds[inside_bound_index].Usage == USAGE_USED);
        ASSERT(m_Bounds[inside_bound_index+1].Usage == USAGE_FREE);
        ASSERT(m_Bounds[inside_bound_index+1].Position > position);
        
        m_Bounds.emplace(m_Bounds.begin() + inside_bound_index + 1, MemoryBound_t{USAGE_FREE, position});
        if(m_Bounds[inside_bound_index + 2].Position == position + size) {
            m_Bounds.erase(m_Bounds.begin() + inside_bound_index + 2);
        } else {
            m_Bounds.emplace(m_Bounds.begin() + inside_bound_index + 2, MemoryBound_t{USAGE_USED, position + size});
        }
        
        if(m_Bounds[inside_bound_index].Position == position) {
            m_Bounds.erase(m_Bounds.begin() + inside_bound_index);
            if(inside_bound_index != 0) {
                m_Bounds.erase(m_Bounds.begin() + inside_bound_index);
            }
        }
        
        if(m_Bounds.size() == 1) {
            m_Bounds.emplace(m_Bounds.begin(), MemoryBound_t{USAGE_USED, 0});
        }
    }

    void Arena::FreeAll() {
        m_Bounds = {{USAGE_USED, 0}, {USAGE_FREE, 0}};

        if(m_pNext) {
            delete m_pNext;
            m_pNext = nullptr;
        }
    }

    void Arena::DebugPrint(int hierarchyIndex)
    {
        Console::LogInfo("%d - Arena :", hierarchyIndex);
        Console::LogInfo(ConsoleMessageFlags_t(MESSAGE_NO_NEW_LINE_BIT | MESSAGE_NO_TIME_CODE_BIT), "Bounds :");
        for(MemoryBound_t b : m_Bounds) {
            Console::LogInfo(ConsoleMessageFlags_t(MESSAGE_NO_NEW_LINE_BIT | MESSAGE_NO_TIME_CODE_BIT), "| (At  %zu) %s ", b.Position, b.Usage == USAGE_USED ? "USED" : "FREE");
        }
        Console::LogInfo(MESSAGE_NO_TIME_CODE_BIT, "|\n");

        if(m_pNext) {
            Console::LogInfo("Contains another arena :");
            m_pNext->DebugPrint(hierarchyIndex + 1);
        }
    }
}