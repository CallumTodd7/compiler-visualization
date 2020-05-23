//
// Created by Callum Todd on 2020/04/14.
//

#ifndef COMPILER_VISUALIZATION_DATA_H
#define COMPILER_VISUALIZATION_DATA_H

#import <ostream>
#include "compiler/Lexer.h"
#include "compiler/AST.h"
#include "compiler/Generator.h"

struct Data {
  enum Type {
    NOOP = 0,
    MODE_CHANGE,
    SPECIFIC,
  };
  friend std::ostream& operator<<(std::ostream& os, const Type& type);

  enum Mode {
    START = 0,
    LEXER,
    PARSER,
    CODE_GEN,
    FINISHED,
    ERROR,
  };
  friend std::ostream& operator<<(std::ostream& os, const Mode& mode);

  enum LexerState {
    LEXER_UNINITIALISED = 0,
    NEW_TOKEN,
    END_OF_FILE,
    UNKNOWN,
    START_NUMBER,
    START_STRING,
    START_ALPHA,
    START_OP,
    END_NUMBER,
    END_STRING,
    END_ALPHA_IDENT,
    END_ALPHA_KEYWORD,
    END_OP,
    WORD_UPDATE,
  };

  enum ParserState {
    PARSER_UNINITIALISED = 0,
    START_NODE,
    END_NODE,
    INSERT_NODE_BETWEEN_CHILD,
    PARAM,
    ADD_CHILD,
    EXPECT,
    EXPECT_PASS,
    EXPECT_FAIL,
    ACCEPT,
    ACCEPT_PASS,
    ACCEPT_FAIL,
  };

  enum CodeGenState {
    CODE_GEN_UNINITIALISED = 0,
    OUTPUT,
    ENTER_NODE,
    EXIT_NODE,
    PUSH_CONSTANT,
    POP_CONSTANT,
    SET_REG,
    SET_LOC,
    SET_REG_AND_LOC,
    MOVE_REG,
    MOVE_CONST,
  };

  Type type = Type::NOOP;
  Mode mode;
  std::string filepath;
  LexerState lexerState = LexerState::LEXER_UNINITIALISED;
  LexerContext lexerContextStart;
  LexerContext lexerContextEnd;

  ParserState parserState = ParserState::PARSER_UNINITIALISED;
  ASTType nodeType = ASTType::UNINITIALISED;
  std::pair<std::string, std::string> param;
  std::string parserChildGroup;
  unsigned long index = 0;
  Token::Type targetToken;
  ASTType targetNodeType = ASTType::UNINITIALISED;

  std::string tokenType;

  unsigned long nodeId = 0;
  unsigned long childNodeId = 0;
  CodeGenState codeGenState = CodeGenState::CODE_GEN_UNINITIALISED;
  std::string id;
  bool isNew;
  Register reg = Register::NONE;
  Location* loc = nullptr;
  Register reg2 = Register::NONE;
  bool keep = false;

  std::string string;
  unsigned long long number;
  char peekedChar = 0;

};


#endif //COMPILER_VISUALIZATION_DATA_H
