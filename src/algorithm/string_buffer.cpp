#include "string_buffer.h"
#include "../error_macros.h"

namespace gigno {

    StringBuffer::StringBuffer(size_t capacity) {
        m_Capacity = capacity;
        m_pData = new char[capacity];
        ASSERT(m_pData != nullptr);
        m_pCurrentWord = m_pData;
        m_pCurrentChar = m_pData - 1;
        m_pEnd = m_pData + capacity;
    }

    StringBuffer::StringBuffer(char *starting_word, size_t starting_word_len, size_t capacity, char current_to_add)
    : StringBuffer(capacity) {
        ASSERT(starting_word_len < capacity);
        if(starting_word_len > 0) {
            memcpy(m_pData, starting_word, starting_word_len);
        }
        m_pCurrentChar += starting_word_len;
        m_pCurrentChar ++;
        *m_pCurrentChar = current_to_add;
    }

    StringBuffer::~StringBuffer() {

        size_t freed_pattern = 0xbaadf00d;
        int i = 0;
        while(i + 4 < m_Capacity) {
            memcpy(m_pData + i, &freed_pattern, 4);
            i += 4;
        }

        delete[] m_pData;

        if(m_pNext) {
            delete m_pNext;
        }
    }

    void StringBuffer::PushChar(char c) {
        if(m_pNext) {
            m_pNext->PushChar(c);
            return;
        }

        if(m_pCurrentChar == m_pEnd) {
            size_t remaining_len = (size_t)(m_pCurrentChar - m_pCurrentWord) + 1 * sizeof(char);
            if(remaining_len == -1) {
                remaining_len = 0;
            }
            m_pNext = new StringBuffer{m_pCurrentWord, remaining_len, m_Capacity, c};
        } else {
            m_pCurrentChar++;
            *m_pCurrentChar = c;
        }

        
    }

    char *StringBuffer::EndWord() {
        if(m_pCurrentChar == m_pEnd) {
            return m_pNext->EndWord();
        }

        char *word_ret = m_pCurrentWord;

        m_pCurrentChar++;
        *m_pCurrentChar = '\0';
        m_pCurrentWord = m_pCurrentChar + 1;

        return word_ret;
    }

}