#include "command_token.h"
#include "../algorithm/my_cstr.h"
#include "../error_macros.h"

namespace gigno {

    CommandToken_t::CommandToken_t(const char *expression) {
        if(strlen(expression) == strspn(expression, " \"")) {
            m_WordCount = 0;
            m_Words = new char *[1];
            m_Words[0] = new char[1];
            m_Words[0][0] = '\0';
            return;
        }

        char * expression_copy;
        size_t expression_length = strlen(expression) + 1;
        expression_copy = new char[expression_length];
        strcpy(expression_copy, expression);

        str_replace_char(expression_copy, '\\', '/');

        str_replace_inquote(expression_copy, ' ', '\\', '"');

        char *token = expression_copy;
        size_t token_count = 0;
        token += strspn(token, " ");


        while(token && *token != '\0') {
            token_count++;
            token+= strspn(token, " \"");
            token = strpbrk(token, " \"");
            if(token) {
                token += strspn(token, " \"");
            }
        }

        m_Words = new char*[token_count];
        token = strtok(expression_copy, " \"");
        int i = 0;
        while(token) {
            size_t tok_size = strlen(token) + 1;
            m_Words[i] = new char[tok_size];
            strcpy(m_Words[i], token);
            token = strtok(nullptr, " \"");
            i++;
        }

        for(int j = 0; j < token_count; j++) {
            str_replace_char(m_Words[j], '\\', ' ');
        }

        m_WordCount = token_count;

        delete[] expression_copy;
    }

    CommandToken_t::~CommandToken_t() {
        for(int i = 0; i < m_WordCount; i++ ) {
            delete[] m_Words[i];
        }
        delete[] m_Words;
    }

    const char * CommandToken_t::GetArg(uint32_t index) const {
        ASSERT_V(m_WordCount > 1, nullptr);
        ASSERT_V(index + 1 < m_WordCount, nullptr);
        return m_Words[index + 1];
    }

    bool CommandToken_t::CompareName(const char *other) const {
        return strcmp(m_Words[0], other) == 0;
    }

    bool CommandToken_t::GetArgInt(uint32_t arg_index, int64_t &output) const noexcept {
        char *end;
        output = strtol(GetArg(arg_index), &end, 10);
        return (end != GetArg(arg_index) && *end == '\0' /*&& errno != ERANGE    ignore out of range !*/);
    }

    bool CommandToken_t::GetArgFloat(uint32_t arg_index, float &output) const noexcept {
        char *end;
        output = strtof(GetArg(arg_index), &end);
        return (end != GetArg(arg_index) && *end == '\0');
    }
}
