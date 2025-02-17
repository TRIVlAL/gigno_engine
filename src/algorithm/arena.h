#ifndef ARENA_H
#define ARENA_H

#include <vector>

namespace gigno {

    /*
    Represents a pre-allocated memory pool from which we can take memory and free it.
        - fast because we make only one call to the OS when first constructing and then the memory is just kept
        - fast because the memory is contiguous and we have more chance of good memory access

        - useful because pointers are NEVER INVALIDATED until a call to Arena::Free() or the deletion of the Arena
    */
    class Arena {
    public:
        /*
        capacity : amound of pre-allocated memory in bytes that will be avaliable in the arena.
        */
        Arena(size_t capactity);
        ~Arena();

        /*
        Returns a pointer to "size" bytes of memory that can be used.
        */
        void *Alloc(size_t size);

        /*
        Frees "size" bytes of memory to be reused afterward.
        */
        void Free(void *position, size_t size);

        /*
        'Frees' all of the used up memory so that it can be reused afterward.
        */
        void FreeAll();
        
        void DebugPrint();
        
    private:

        void Free(size_t position, size_t size);

        enum MemoryUsage_t {
            MEM_FREE,
            MEM_USED
        };
        
        struct MemoryBound_t {
            MemoryUsage_t Usage;
            size_t Position;
        };

        /*
        Number of bytes avaliable in the arena.
        */
        size_t m_Capactity;

        /*
        Pointer to the pre-allocated data.
        */
        char *m_pData;

        /*
        Sorted by acending position. The memory from a given position (including) up to the next one in the vector (or the end) is
        USED/FREE. 
            Example :
            {{0 : FREE}, {280, USED}, {320, FREE}} means that the whole memory is free except for bytes 280 to 329 including.
            {{0 : USED}, {0, FREE}} means that the whole memory is free
            {{0 : USED}, {1024, FREE}} means that bytes 0- 1023 are used and the rest up to the end is free
            etc...
        */
        std::vector<MemoryBound_t> m_Bounds;
    };

}

#endif