#ifndef STRING_BUFFER
#define STRING_BUFFER

#include <memory.h>

namespace gigno {

    /*
    Buffer allowing to 'build' a string character by character. 
    Strings allocated with StringBuffer are valid until destruction of said StringBuffer, and are null-terminated.
    capacity is allocated by default, and a new buffer of capacity will be added every time you run out of space. 
    */
    class StringBuffer {
    public:
        StringBuffer(size_t capacity);
        ~StringBuffer();

        // Adds the character to the string burrently built.
        void PushChar(char c);
        // Returns the string currently building. Automatically adds null-termination.
        char *EndWord();
    private:
        StringBuffer(char *starting_word, size_t starrting_word_len, size_t capacity, char current_to_add);

        char *m_pCurrentChar; //points to the last written character.
        char *m_pCurrentWord;
        char *m_pEnd;

        StringBuffer *m_pNext = nullptr;

        char *m_pData;
        size_t m_Capacity;
    };

}

#endif