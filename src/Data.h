//
// Created by Callum Todd on 2020/04/14.
//

#ifndef COMPILER_VISUALIZATION_DATA_H
#define COMPILER_VISUALIZATION_DATA_H

#import <ostream>

struct Data {
  enum Type {
    NOOP,
    MODE_CHANGE,
  };
  friend std::ostream& operator<<(std::ostream& os, const Type& type);
  enum Mode {
    LEXER,
    PARSER,
    CODE_GEN,
    FINISHED,
  };
  friend std::ostream& operator<<(std::ostream& os, const Mode& mode);

  Type type = Type::NOOP;
  Mode mode;

};


#endif //COMPILER_VISUALIZATION_DATA_H
