//
// Created by Callum Todd on 2020/04/14.
//

#ifndef COMPILER_VISUALIZATION_DATA_H
#define COMPILER_VISUALIZATION_DATA_H

#import <ostream>
#include "compiler/Lexer.h"

struct Data {
  enum Type {
    NOOP = 0,
    MODE_CHANGE,
    SPECIFIC,
  };
  friend std::ostream& operator<<(std::ostream& os, const Type& type);

  enum Mode {
    LEXER,
    PARSER,
    CODE_GEN,
    FINISHED,
  };
  friend std::ostream& operator<<(std::ostream& os, const Mode& mode);

  enum LexerState {
    UNINITIALISED = 0,
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

  Type type = Type::NOOP;
  Mode mode;
  std::string filepath;
  LexerState lexerState = LexerState::UNINITIALISED;
  LexerContext lexerContextStart;
  LexerContext lexerContextEnd;

  std::string string;
  unsigned long long number;
  char peekedChar = 0;

};


#endif //COMPILER_VISUALIZATION_DATA_H
