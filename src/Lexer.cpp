//
// Created by Callum Todd on 2020/02/14.
//

#include <iostream>
#include "Lexer.h"

Lexer::Lexer(const std::string& sourceFilepath) {
  auto fileStream = new std::ifstream(sourceFilepath);
  file = new InputFile{
      .filepath = sourceFilepath,
      .fileStream = fileStream
  };

  // Load first char
  advanceCursor();
}

Lexer::~Lexer() {
  delete file->fileStream;
}

std::vector<Token> Lexer::getTokenStream() {
  std::vector<Token> tokens;

  while (true) {
    Token token = nextToken();
    if (token.type == Token::Type::TOKEN_EOF) break;
    if (token.type == Token::Type::TOKEN_ERROR) break; // TODO throw exception instead
    tokens.push_back(token);
  }

  return tokens;
}

LexerContext Lexer::getContext() {
  return LexerContext{
      .file = file,
      .lineNumber = currentLine,
      .characterPos = currentLinePos
  };
}

Token Lexer::nextToken() {
  LexerContext lexerContext = getContext();

  // Skip whitespace and comments
  skipWhitespaceAndComments(lexerContext);

  // Get new context after skipping whitespace
  lexerContext = getContext();

  // Check for EOF
  if (currentChar < 0 && file->fileStream->eof()) {
    return Token::eofToken(lexerContext);
  }

  if (isdigit(currentChar)) {
    return readNumber(lexerContext);
  } else if (isalpha(currentChar)) {
    std::string word;

    // Read alpha in
    do {
      word.push_back(currentChar);
      advanceCursor();
    } while (isalpha(currentChar) || isdigit(currentChar)); // Digits can be in alpha after the first char

    // Is keyword?
    Token keywordToken = identifyKeyword(lexerContext, word);
    if (keywordToken.type != Token::Type::TOKEN_ERROR) {
      return keywordToken;
    }

    // Must be identifier
    return {lexerContext, Token::Type::TOKEN_IDENTIFIER, word};
  } else {
    Token operatorToken = readOperator(lexerContext);
    if (operatorToken.type != Token::Type::TOKEN_ERROR) {
      return operatorToken;
    }
  }

  // Error
  std::cout << lexerContext << " Lexer: Unknown character(s): " << currentChar << std::endl;
  return Token::errorToken(lexerContext);
}

void Lexer::skipWhitespaceAndComments(const LexerContext& lexerContext) {
  while (isWhitespace(currentChar) || (currentChar == '/' && (peekChar() == '/' || peekChar() == '*'))) {
    // - Skip whitespace
    while (isWhitespace(currentChar)) {
      advanceCursor();
    }

    // - Comments
    // - Single line comment
    if (currentChar == '/' && peekChar() == '/') {
      advanceCursor(); // Eat '/'
      advanceCursor(); // Eat '/'

      while (currentChar != '\n' && !file->fileStream->eof()) {
        advanceCursor();
      }
      advanceCursor(); // Eat '\n'
    }
    // - Multi-line comment
    if (currentChar == '/' && peekChar() == '*') {
      advanceCursor(); // Eat '/'
      advanceCursor(); // Eat '*'

      int nestedLevel = 1;
      while (true) {
        if (currentChar == '*' && peekChar() == '/') {
          // End of this comment
          advanceCursor(); // Eat '*'
          advanceCursor(); // Eat '/'

          nestedLevel--;

          // Have all comments ended?
          if (nestedLevel <= 0) {
            break;
          }
        } else if (currentChar == '/' && peekChar() == '*') {
          // Start of new nested comment
          advanceCursor(); // Eat '/'
          advanceCursor(); // Eat '*'

          nestedLevel++;
        } else if (currentChar == '/' && peekChar() == '/') {
          // Without this branch the'//' will sometimes allow for the skipping of the comment end
          // Example:
          // ```
          //   /* muti-line comment with a single line comment directly before the end
          //   //*/
          // ```
          advanceCursor(); // Eat first '/', the second '/' will be taken care of by the `advanceCursor()` below
        }

        if (file->fileStream->eof()) {
          std::cout << lexerContext << " Nested comment failed to close before end of file" << std::endl;
          return;
        }

        // Get next char in comment
        advanceCursor();
      }
    }
  }
}

Token Lexer::readNumber(const LexerContext& lexerContext) {
  // Clear leading zeros
  int leadingZeros = 0;
  while (currentChar == '0') {
    leadingZeros++;
    advanceCursor();
  }

  // Get string from chars
  std::string word;

  while (isdigit(currentChar)) {
    // Add char
    word.push_back(currentChar);
    advanceCursor();
  }

  // Test for zero
  if (word.empty()) {
    if (leadingZeros > 0) {
      // Must be a zero
      return {lexerContext, Token::Type::TOKEN_INTEGER, 0};
    } else {
      // Error because there's no number here
      std::cout << lexerContext << " Lexer: Number not zero and not 1-9. Huh?" << std::endl;
      return Token::errorToken(lexerContext);
    }
  }

  // Parse number
  errno = 0; // reset errno

  unsigned long long parsedNumber = strtoull(word.c_str(), nullptr, 10);

  // Handle errors
  if (parsedNumber == 0) {
    std::cout << lexerContext << " Lexer: Number not valid (prob internal?)." << std::endl;
    return Token::errorToken(lexerContext);
  } else if (parsedNumber == LONG_MIN && errno == ERANGE) {
    std::cout << lexerContext << " Lexer: Number out of range. Too low." << std::endl;
    return Token::errorToken(lexerContext);
  } else if (parsedNumber == LONG_MAX && errno == ERANGE) {
    std::cout << lexerContext << " Lexer: Number out of range. Too high." << std::endl;
    return Token::errorToken(lexerContext);
  } else if (errno != 0) {
    std::cout << lexerContext << " Lexer: Unknown error parsing number." << std::endl;
    return Token::errorToken(lexerContext);
  }

  return {lexerContext, Token::Type::TOKEN_INTEGER, parsedNumber};
}

Token Lexer::identifyKeyword(const LexerContext& lexerContext, const std::string& keyword) {
  if (keyword == "u8") {
    return {lexerContext, Token::Type::TOKEN_KEYWORD_U8};
  } else if (keyword == "s8") {
    return {lexerContext, Token::Type::TOKEN_KEYWORD_S8};
  } else if (keyword == "void") {
    return {lexerContext, Token::Type::TOKEN_KEYWORD_VOID};
  } else if (keyword == "return") {
    return {lexerContext, Token::Type::TOKEN_KEYWORD_RETURN};
  } else if (keyword == "break") {
    return {lexerContext, Token::Type::TOKEN_KEYWORD_BREAK};
  } else if (keyword == "if") {
    return {lexerContext, Token::Type::TOKEN_KEYWORD_IF};
  } else if (keyword == "else") {
    return {lexerContext, Token::Type::TOKEN_KEYWORD_ELSE};
  } else if (keyword == "while") {
    return {lexerContext, Token::Type::TOKEN_KEYWORD_WHILE};
  } else if (keyword == "func") {
    return {lexerContext, Token::Type::TOKEN_KEYWORD_PROCEDURE};
  }

  return Token::errorToken(lexerContext);
}

Token Lexer::readOperator(const LexerContext& lexerContext) {
  char firstChar = currentChar;
  advanceCursor();

  if (firstChar == ';') {
    return {lexerContext, Token::Type::TOKEN_SEMICOLON};
  } else if (firstChar == ':') {
    return {lexerContext, Token::Type::TOKEN_COLON};
  } else if (firstChar == '+') {
    return {lexerContext, Token::Type::TOKEN_ADD};
  } else if (firstChar == '-') {
    return {lexerContext, Token::Type::TOKEN_MINUS};
  } else if (firstChar == '*') {
    return {lexerContext, Token::Type::TOKEN_MULTIPLY};
  } else if (firstChar == '/') {
    return {lexerContext, Token::Type::TOKEN_DIVIDE};
  } else if (firstChar == '(') {
    return {lexerContext, Token::Type::TOKEN_PARENTHESIS_OPEN};
  } else if (firstChar == ')') {
    return {lexerContext, Token::Type::TOKEN_PARENTHESIS_CLOSE};
  } else if (firstChar == '{') {
    return {lexerContext, Token::Type::TOKEN_BRACE_OPEN};
  } else if (firstChar == '}') {
    return {lexerContext, Token::Type::TOKEN_BRACE_CLOSE};
  } else if (firstChar == '=') {
    return {lexerContext, Token::Type::TOKEN_EQUALS};
  } else if (firstChar == '!' && currentChar == '=') {
    advanceCursor();
    return {lexerContext, Token::Type::TOKEN_NOT_EQUALS}; // '!='
  } else if (firstChar == '&' && currentChar == '&') {
    advanceCursor();
    return {lexerContext, Token::Type::TOKEN_LOGICAL_AND}; // '&&'
  } else if (firstChar == '|' && currentChar == '|') {
    advanceCursor();
    return {lexerContext, Token::Type::TOKEN_LOGICAL_OR}; // '||'
  } else if (firstChar == '<') {
    if (currentChar == '=') {
      advanceCursor();
      return {lexerContext, Token::Type::TOKEN_LESS_THAN_OR_EQUAL}; // '<='
    } else {
      return {lexerContext, Token::Type::TOKEN_LESS_THAN}; // '<'
    }
  } else if (firstChar == '>') {
    if (currentChar == '=') {
      advanceCursor();
      return {lexerContext, Token::Type::TOKEN_GREATER_THAN_OR_EQUAL}; // '>='
    } else {
      return {lexerContext, Token::Type::TOKEN_GREATER_THAN}; // '>'
    }
  } else if (firstChar == ',') {
    return {lexerContext, Token::Type::TOKEN_COMMA};
  } else if (firstChar == '.') {
    return {lexerContext, Token::Type::TOKEN_DOT};
  }

  return Token::errorToken(lexerContext);
}

void Lexer::advanceCursor() {
  if (!peekedChars.empty()) {
    currentChar = peekedChars.front();
    peekedChars.pop();
  } else {
    currentChar = file->fileStream->get();
  }

  currentLinePos++;
  if (currentChar == '\n' || currentChar == EOF_CHAR) {
    totalCharacters += currentLinePos;
  }
  if (currentChar == '\n') {
    currentLinePos = 0;
    currentLine++;
  }
}

char Lexer::peekChar(unsigned int positionAheadOfCursor) {
  assert(positionAheadOfCursor > 0);

  // Need to peek more chars?
  if (positionAheadOfCursor >= peekedChars.size()) {
    unsigned int requiredChars = positionAheadOfCursor - peekedChars.size();

    for (unsigned int i = 0; i < requiredChars; ++i) {
      peekedChars.push(file->fileStream->get());
    }
  }

  return *(&peekedChars.front() + (positionAheadOfCursor - 1));
}

bool Lexer::isWhitespace(char c) {
  return c == ' ' ||
      c == '\t' ||
      c == '\n';
}
