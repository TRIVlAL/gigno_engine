#include "arena.h"
#include "../error_macros.h"
#include "../debug/console/console.h"

namespace gigno {

    Arena::Arena(size_t capactity) {
        ASSERT(capactity > 0);
        m_Capactity = capactity;
        m_pData = (char*)malloc(capactity);
        m_Bounds = {{MEM_USED, 0},{MEM_FREE, 0}};
    }

    Arena::~Arena() {
        free(m_pData);
    }

    void *Arena::Alloc(size_t size) {
        int i = m_Bounds.size();
        while(--i >= 0) {
            if(m_Bounds[i].Usage == MEM_FREE) {
                size_t avaliable_space{};

                if(i == m_Bounds.size() - 1) {
                    avaliable_space = m_Capactity - m_Bounds[i].Position;
                } else {
                    avaliable_space = m_Bounds[i + 1].Position - m_Bounds[i].Position;
                }

                if(avaliable_space >= size)  {
                    void *ret = &m_pData[m_Bounds[i].Position];

                    m_Bounds[i].Position += size;
                    if(m_Bounds[i].Position == m_Bounds[i+1].Position) {
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
        ASSERT_V(false, nullptr);
        return nullptr;
    }

    void Arena::Free(void *position, size_t size) {
        size_t index_position = (char*)position - m_pData;

        ASSERT(index_position < m_Capactity);

        Free(index_position, size);
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
        
        ASSERT(m_Bounds[inside_bound_index].Usage == MEM_USED);
        ASSERT(m_Bounds[inside_bound_index+1].Usage == MEM_FREE);
        ASSERT(m_Bounds[inside_bound_index+1].Position > position);
        
        m_Bounds.emplace(m_Bounds.begin() + inside_bound_index + 1, MemoryBound_t{MEM_FREE, position});
        if(m_Bounds[inside_bound_index + 2].Position == position + size) {
            m_Bounds.erase(m_Bounds.begin() + inside_bound_index + 2);
        } else {
            m_Bounds.emplace(m_Bounds.begin() + inside_bound_index + 2, MemoryBound_t{MEM_USED, position + size});
        }
        
        if(m_Bounds[inside_bound_index].Position == position) {
            m_Bounds.erase(m_Bounds.begin() + inside_bound_index);
            if(inside_bound_index != 0) {
                m_Bounds.erase(m_Bounds.begin() + inside_bound_index);
            }
        }
        
        if(m_Bounds.size() == 1) {
            m_Bounds.emplace(m_Bounds.begin(), MemoryBound_t{MEM_USED, 0});
        }
    }

    void Arena::FreeAll() {
        m_Bounds = {{MEM_USED, 0}, {MEM_FREE, 0}};
    }

    void Arena::DebugPrint()
    {
        Console::LogInfo("Arena :");
        Console::LogInfo(ConsoleMessageFlags_t(MESSAGE_NO_NEW_LINE_BIT | MESSAGE_NO_TIME_CODE_BIT), "Bounds :");
        for(MemoryBound_t b : m_Bounds) {
            Console::LogInfo(ConsoleMessageFlags_t(MESSAGE_NO_NEW_LINE_BIT | MESSAGE_NO_TIME_CODE_BIT), "| (At  %zu) %s ", b.Position, b.Usage == MEM_USED ? "USED" : "FREE");
        }
        Console::LogInfo(MESSAGE_NO_TIME_CODE_BIT, "|\n");
    }
}