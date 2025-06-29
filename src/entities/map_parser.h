#ifndef MAP_PARSER_H
#define MAP_PARSER_H

#include <fstream>
#include <vector>
#include "../utils/string_buffer.h"

namespace gigno {

    class MapParser {
    public:
        enum CommandType_t {
            MAP_COMMAND_CREATE_ENTITY,
            MAP_COMMAND_SET_KEY_VALUE
        };

        struct MapCommand_t {
            MapCommand_t(CommandType_t t, char *tn) :
                Type(t), TypeName{tn} {}

            CommandType_t Type;

            char *TypeName;
            char *Values[10]; // set if Type == COMMAND_SET_KEY_VALUE
            size_t ValueCount{};
        };

        std::vector<MapCommand_t> operator()(const char *filepath);

    private :

        enum TokenType_t {
            MAP_TOKEN_NONE,
            MAP_TOKEN_OPEN_BRACE,
            MAP_TOKEN_CLOSE_BRACE,
            MAP_TOKEN_QUOTE,
            MAP_TOKEN_COLON,
            MAP_TOKEN_COMMA,
            MAP_TOKEN_STRING_EXPLICIT,
            MAP_TOKEN_STRING_IMPLICIT,
        };

        struct Token_t {
            Token_t(TokenType_t t, char *dat, size_t num) :
                Type{t}, Data{dat}, LineNumber{num} {}

            TokenType_t Type;

            char *Data{}; //set if Type == TOKEN_STRING

            size_t LineNumber;
        };

        std::vector<Token_t> Lex(std::ifstream &filestream);
        std::vector<MapCommand_t> ToCommandList(const std::vector<Token_t> &tokens, const char *filepath);

        bool IsWhitespace(char c);
        bool IsSpecial(char c);

        const char *TokenTypeToString(TokenType_t t);

        StringBuffer m_Strings{2048};
    };

}

#endif