//
// Created by Callum Todd on 2020/04/14.
//

#import "Data.h"

std::ostream& operator<<(std::ostream& os, const Data::Type& type) {
  switch (type) {
    case Data::Type::NOOP:
      os << "NOOP";
      break;
    case Data::Type::MODE_CHANGE:
      os << "MODE_CHANGE";
      break;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Data::Mode& mode) {
  switch (mode) {
    case Data::Mode::LEXER:
      os << "LEXER";
      break;
    case Data::Mode::PARSER:
      os << "PARSER";
      break;
    case Data::Mode::CODE_GEN:
      os << "CODE_GEN";
      break;
    case Data::Mode::FINISHED:
      os << "FINISHED";
      break;
  }
  return os;
}
