//
// Created by Callum Todd on 2020/02/14.
//

#ifndef COMPILER_VISUALIZATION_LEXER_H
#define COMPILER_VISUALIZATION_LEXER_H

#include <string>
#include <fstream>
#include <vector>
#include <queue>

#define EOF_CHAR -1

struct InputFile {
  std::string filepath;
  std::ifstream* fileStream;
};

struct LexerContext {
  InputFile* file = nullptr;
  int lineNumber = -1;
  int characterPos = -1;

  friend std::ostream& operator<<(std::ostream& os, const LexerContext& context);
};

struct Token {
  enum Type {
    TOKEN_ERROR = 0,
    TOKEN_EOF = 1,

    TOKEN_IDENTIFIER = 2,
    TOKEN_STRING = 3,
    TOKEN_INTEGER = 4,

    TOKEN_SEMICOLON = 5,
    TOKEN_COLON = 6,
    TOKEN_ADD = 7,
    TOKEN_MINUS = 8,
    TOKEN_MULTIPLY = 9,
    TOKEN_DIVIDE = 10,
    TOKEN_PARENTHESIS_OPEN = 11,
    TOKEN_PARENTHESIS_CLOSE = 12,
    TOKEN_BRACE_OPEN = 13,
    TOKEN_BRACE_CLOSE = 14,
    TOKEN_ASSIGN = 15,
    TOKEN_EQUALS = 16,
    TOKEN_NOT_EQUALS = 17,
    TOKEN_LOGICAL_AND = 18,
    TOKEN_LOGICAL_OR = 19,
    TOKEN_LOGICAL_NOT = 36,
    TOKEN_LESS_THAN = 20,
    TOKEN_LESS_THAN_OR_EQUAL = 21,
    TOKEN_GREATER_THAN = 22,
    TOKEN_GREATER_THAN_OR_EQUAL = 23,
    TOKEN_COMMA = 24,
    TOKEN_DOT = 25,

    TOKEN_KEYWORD_U8 = 26,
    TOKEN_KEYWORD_S8 = 27,
    TOKEN_KEYWORD_VOID = 28,
    TOKEN_KEYWORD_RETURN = 29,
    TOKEN_KEYWORD_CONTINUE = 30,
    TOKEN_KEYWORD_BREAK = 31,
    TOKEN_KEYWORD_IF = 32,
    TOKEN_KEYWORD_ELSE = 33,
    TOKEN_KEYWORD_WHILE = 34,
    TOKEN_KEYWORD_FOR = 35,
    TOKEN_KEYWORD_EXTERN = 37,
  };

  enum class ValueType {
    NONE = 0,
    STRING,
    INTEGER,
  };

  LexerContext lexerContext;

  Type type;

  ValueType valueType = ValueType::NONE;
  union {
    unsigned long long integerValue;

    struct {
      int count;
      char* data;
    } stringValue{};
  } value;

  Token(LexerContext lexerContext, Type type) {
    this->lexerContext = lexerContext;
    this->type = type;
  }

  Token(LexerContext lexerContext, Type type, const std::string& string) {
    this->lexerContext = lexerContext;
    this->type = type;
    this->valueType = ValueType::STRING;
    this->value.stringValue.count = string.size();
    this->value.stringValue.data = new char[this->value.stringValue.count]; //@leak TODO replace string values with global ident/string store
    strcpy(this->value.stringValue.data, string.c_str());
  }

  Token(LexerContext lexerContext, Type type, unsigned long long number) {
    this->lexerContext = lexerContext;
    this->type = type;
    this->valueType = ValueType::INTEGER;
    this->value.integerValue = number;
  }

  static Token errorToken(LexerContext lexerContext) {
    return Token(lexerContext, Type::TOKEN_ERROR);
  };

  static Token eofToken(LexerContext lexerContext) {
    return Token(lexerContext, Type::TOKEN_EOF);
  };

  friend std::ostream& operator<<(std::ostream& os, const Token& token);
  friend std::ostream& operator<<(std::ostream& os, const Token::Type& type);

};

class Lexer {
private:
  InputFile* file;

  std::queue<char> peekedChars;

  long totalCharacters = 0;
  int currentLine = 1;
  int currentLinePos = 0;
  char currentChar = (char) 0;

public:
  explicit Lexer(const std::string& sourceFilepath);
  ~Lexer();

  std::vector<Token> getTokenStream();

private:
  Token nextToken();

  void advanceCursor();
  char peekChar(unsigned int positionAheadOfCursor = 1);

  bool skipWhitespaceAndComments(const LexerContext& lexerContext);
  Token readNumber(const LexerContext& lexerContext);
  static Token identifyKeyword(const LexerContext& lexerContext, const std::string& keyword);
  Token readOperator(const LexerContext& lexerContext);

  static bool isWhitespace(char c);

  LexerContext getContext();
};


#endif //COMPILER_VISUALIZATION_LEXER_H
