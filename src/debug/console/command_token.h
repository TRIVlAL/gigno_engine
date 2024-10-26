#ifndef COMMAND_TOKEN_H
#define COMMAND_TOKEN_H

#include <stdint.h>

namespace gigno {

    struct CommandToken_t {
    public:

        CommandToken_t(const char *expression);
        ~CommandToken_t();

        const char *GetName() const {
            return m_WordCount > 0 ? m_Words[0] : nullptr; 
        }
        bool CompareName(const char *other) const;

        uint32_t GetArgC() const { return m_WordCount - 1; }

        const char *GetArg(uint32_t index) const;

        bool GetArgInt(uint32_t arg_index, int &output) const noexcept;
        bool GetArgFloat(uint32_t arg_index, float &output) const noexcept;

    private:
        uint32_t m_WordCount{};
        char **m_Words{};
    };

}

#endif