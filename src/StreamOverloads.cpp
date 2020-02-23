//
// Created by Callum Todd on 2020/02/15.
//

#include "Lexer.h"

std::ostream& operator<<(std::ostream& os, const Token::Type& type) {
  switch (type) {
    case Token::Type::TOKEN_ERROR:
      os << "ERROR";
      break;
    case Token::Type::TOKEN_EOF:
      os << "EOF";
      break;
    case Token::Type::TOKEN_IDENTIFIER:
      os << "IDENTIFIER";
      break;
    case Token::Type::TOKEN_STRING:
      os << "STRING";
      break;
    case Token::Type::TOKEN_INTEGER:
      os << "INTEGER";
      break;
    case Token::Type::TOKEN_SEMICOLON:
      os << "SEMICOLON";
      break;
    case Token::Type::TOKEN_COLON:
      os << "COLON";
      break;
    case Token::Type::TOKEN_ADD:
      os << "ADD";
      break;
    case Token::Type::TOKEN_MINUS:
      os << "MINUS";
      break;
    case Token::Type::TOKEN_MULTIPLY:
      os << "MULTIPLY";
      break;
    case Token::Type::TOKEN_DIVIDE:
      os << "DIVIDE";
      break;
    case Token::Type::TOKEN_PARENTHESIS_OPEN:
      os << "PARENTHESIS_OPEN";
      break;
    case Token::Type::TOKEN_PARENTHESIS_CLOSE:
      os << "PARENTHESIS_CLOSE";
      break;
    case Token::Type::TOKEN_BRACE_OPEN:
      os << "BRACE_OPEN";
      break;
    case Token::Type::TOKEN_BRACE_CLOSE:
      os << "BRACE_CLOSE";
      break;
    case Token::Type::TOKEN_ASSIGN:
      os << "ASSIGN";
      break;
    case Token::Type::TOKEN_EQUALS:
      os << "EQUALS";
      break;
    case Token::Type::TOKEN_NOT_EQUALS:
      os << "NOT_EQUALS";
      break;
    case Token::Type::TOKEN_LOGICAL_AND:
      os << "LOGICAL_AND";
      break;
    case Token::Type::TOKEN_LOGICAL_OR:
      os << "LOGICAL_OR";
      break;
    case Token::Type::TOKEN_LESS_THAN:
      os << "LESS_THAN";
      break;
    case Token::Type::TOKEN_LESS_THAN_OR_EQUAL:
      os << "LESS_THAN_OR_EQUAL";
      break;
    case Token::Type::TOKEN_GREATER_THAN:
      os << "GREATER_THAN";
      break;
    case Token::Type::TOKEN_GREATER_THAN_OR_EQUAL:
      os << "GREATER_THAN_OR_EQUAL";
      break;
    case Token::Type::TOKEN_COMMA:
      os << "COMMA";
      break;
    case Token::Type::TOKEN_DOT:
      os << "DOT";
      break;
    case Token::Type::TOKEN_KEYWORD_U8:
      os << "KEYWORD_U8";
      break;
    case Token::Type::TOKEN_KEYWORD_S8:
      os << "KEYWORD_S8";
      break;
    case Token::Type::TOKEN_KEYWORD_VOID:
      os << "KEYWORD_VOID";
      break;
    case Token::Type::TOKEN_KEYWORD_RETURN:
      os << "KEYWORD_RETURN";
      break;
    case Token::Type::TOKEN_KEYWORD_CONTINUE:
      os << "KEYWORD_CONTINUE";
      break;
    case Token::Type::TOKEN_KEYWORD_BREAK:
      os << "KEYWORD_BREAK";
      break;
    case Token::Type::TOKEN_KEYWORD_IF:
      os << "KEYWORD_IF";
      break;
    case Token::Type::TOKEN_KEYWORD_ELSE:
      os << "KEYWORD_ELSE";
      break;
    case Token::Type::TOKEN_KEYWORD_WHILE:
      os << "KEYWORD_WHILE";
      break;
    case Token::Type::TOKEN_KEYWORD_FOR:
      os << "KEYWORD_FOR";
      break;
    default:
      os << "ERROR_TYPE_HAS_NO_STRING_VALUE";
      break;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Token& token) {
  os << token.lexerContext << " " << token.type;
  switch (token.valueType) {
    case Token::ValueType::NONE:
      break;
    case Token::ValueType::STRING:
      os << "(" << std::string(token.value.stringValue.data, token.value.stringValue.count) << ")";
      break;
    case Token::ValueType::INTEGER:
      os << "(" << token.value.integerValue << ")";
      break;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const LexerContext& context) {
  if (context.file) {
    os << context.file->filepath;
  } else {
    os << "NULL";
  }
  os << ":" << context.lineNumber << ":" << context.characterPos;
  return os;
}
