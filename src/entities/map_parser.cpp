#include "map_parser.h"
#include "../application.h"

namespace gigno {

    std::vector<MapParser::MapCommand_t> MapParser::operator()(const char *filepath) {

        std::ifstream file{filepath};
        if(file.is_open()) {
            std::vector<Token_t> tokens{Lex(file)};
            return ToCommandList(tokens, filepath);
        } else {
            Console::LogInfo("Failed to open map file '%s'", filepath);
            return std::vector<MapParser::MapCommand_t>{};
        }
    }

    std::vector<MapParser::Token_t> MapParser::Lex(std::ifstream &filestream) {

        std::vector<Token_t> tokens{};
        char current{};
        size_t current_line{};

        filestream.get(current);

        while((bool)filestream) {

            if(current == '{') {
                tokens.emplace_back(MAP_TOKEN_OPEN_BRACE, nullptr, current_line);
            } else if (current == '}') {
                tokens.emplace_back(MAP_TOKEN_CLOSE_BRACE, nullptr, current_line);
            } else if(current == ':') {
                tokens.emplace_back(MAP_TOKEN_COLON, nullptr, current_line);
            } else if(current == ',') {
                tokens.emplace_back(MAP_TOKEN_COMMA, nullptr, current_line);
            } else if(current == '"') {
                //tokens.emplace_back(MAP_TOKEN_QUOTE, nullptr, current_line);

                filestream.get(current);

                size_t count = 0;

                while(current != '\0' && current != '"' && (bool)filestream) {
                    m_Strings.PushChar(current);
                    filestream.get(current);
                }
                tokens.emplace_back(MAP_TOKEN_STRING_EXPLICIT, m_Strings.EndWord(), current_line);
                count++;

                if(current == '"' && (bool)filestream) {
                    //tokens.emplace_back(MAP_TOKEN_QUOTE, nullptr, current_line);
                    filestream.get(current);
                }
                continue;
            } else if(IsWhitespace(current)) {
                if(current == '\n') {
                    current_line++;
                }
            } else {
                //not a special/whitespace -> take it implicitly as a string

                while(current != '\0' && current != std::char_traits<char>::eof() && !IsSpecial(current) && !IsWhitespace(current) && (bool)filestream) {
                    if(current == '\\') {
                        filestream.get(current);
                        if(current == '\0' || current == std::char_traits<char>::eof() || !(bool)filestream) {
                            break;
                        }
                    }
                    m_Strings.PushChar(current);
                    filestream.get(current);
                }

                tokens.emplace_back(MAP_TOKEN_STRING_IMPLICIT, m_Strings.EndWord(), current_line);

                // do not increment current because we want to handle the upcomming special.
                continue;
            }

            filestream.get(current);
        }

        return tokens;
    }

    #define UNEXPECTED_TOKEN(tok, last_tok, expected) Console::LogWarning("PARSER : Unexpected %s %s following %s %s. Expected " expected "! (%s line %u) ", TokenTypeToString(tok.Type),\
                            tok.Type != MAP_TOKEN_STRING_EXPLICIT ? tok.Type != MAP_TOKEN_STRING_IMPLICIT ? " " : tok.Data : tok.Data, \
                            TokenTypeToString(last_tok.Type), \
                            tok.Type != MAP_TOKEN_STRING_EXPLICIT ? last_tok.Type != MAP_TOKEN_STRING_IMPLICIT ? " " : last_tok.Data : last_tok.Data, \
                            filepath, tok.LineNumber);

    std::vector<MapParser::MapCommand_t> MapParser::ToCommandList(const std::vector<Token_t> &tokens, const char *filepath) {
        std::vector<MapCommand_t> commands;

        const char *unexpected_end_file_expected_token = "anything";

        size_t i = 0;
        while(i < tokens.size()) {
            if(tokens[i].Type == MAP_TOKEN_STRING_EXPLICIT || tokens[i].Type == MAP_TOKEN_STRING_IMPLICIT) {
                if(i == 0 || (i > 0 && tokens[i-1].Type == MAP_TOKEN_CLOSE_BRACE)) {
                    commands.emplace_back(MAP_COMMAND_CREATE_ENTITY, tokens[i].Data);
                    i++;
                } 
                else if (tokens[i-1].Type == MAP_TOKEN_OPEN_BRACE || tokens[i-1].Type == MAP_TOKEN_COMMA) {
                    MapCommand_t &current_command = commands.emplace_back(MAP_COMMAND_SET_KEY_VALUE, tokens[i].Data);
                    
                    i++;
                    if(i == tokens.size()) {
                        unexpected_end_file_expected_token = "COLLON ':'";
                        goto unexpected_end_file;
                    }
                    
                    if(tokens[i].Type != MAP_TOKEN_COLON) {
                        UNEXPECTED_TOKEN(tokens[i], tokens[i-1],"collon :");
                        i++;
                        continue;
                    }
                    
                    i++;
                    if(i == tokens.size()) {
                        unexpected_end_file_expected_token = "STRING";
                        goto unexpected_end_file;
                    } 
                    
                    if(tokens[i].Type != MAP_TOKEN_STRING_EXPLICIT && tokens[i].Type != MAP_TOKEN_STRING_IMPLICIT) {
                        UNEXPECTED_TOKEN(tokens[i], tokens[i-1],"'STRING'");
                        i++;
                        continue;
                    }

                    size_t value_index = 0;
                    while(tokens[i].Type == MAP_TOKEN_STRING_EXPLICIT || tokens[i].Type == MAP_TOKEN_STRING_IMPLICIT) {
                        if(value_index < 10) {
                            current_command.Values[value_index] = tokens[i].Data;
                            current_command.ValueCount++;
                        }
                        value_index++;

                        i++;
                        if(i == tokens.size()) {
                            unexpected_end_file_expected_token = "COMMA ','";
                            goto unexpected_end_file;
                        }
                    }

                    if(tokens[i].Type != MAP_TOKEN_COMMA && tokens[i].Type != MAP_TOKEN_CLOSE_BRACE) {
                        UNEXPECTED_TOKEN(tokens[i], tokens[i - 1], "',' or '}'");
                        i++;
                        continue;
                    }
                    i++;

                } else {
                    UNEXPECTED_TOKEN(tokens[i], tokens[i - 1], "other");
                    i++;
                    continue;
                }
            } else {
                i++;
            }
        }

        return commands;

        unexpected_end_file:
        Console::LogInfo("PARSER : Unexpected End File after '%s'. Expected %s (%s Line %u)", TokenTypeToString(tokens[i-1].Type), unexpected_end_file_expected_token, filepath, tokens[i-1].LineNumber);

        return commands;
    }

    bool MapParser::IsWhitespace(char c) {
        return c == ' ' || c == '\n' || c == '\t';
    }

    bool MapParser::IsSpecial(char c) {
        return c == '{' || c == '}' || c == ':' || c == '"';
    }

    const char *MapParser::TokenTypeToString(TokenType_t t) {
        switch(t) {
            case MAP_TOKEN_NONE:
                return "NONE";
            case MAP_TOKEN_OPEN_BRACE:
                return "{";
            case MAP_TOKEN_CLOSE_BRACE:
                return "}";
            case MAP_TOKEN_QUOTE:
                return "\"";
            case MAP_TOKEN_COLON:
                return ":";
            case MAP_TOKEN_COMMA:
                return ",";
            case MAP_TOKEN_STRING_EXPLICIT:
                return "STRING";
            case MAP_TOKEN_STRING_IMPLICIT:
                return "STRING (implicit)";
        }
        return "UNKNOWN";
    }
}
