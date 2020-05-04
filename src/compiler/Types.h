//
// Created by Callum Todd on 2020/02/21.
//

#ifndef COMPILER_VISUALIZATION_TYPES_H
#define COMPILER_VISUALIZATION_TYPES_H

enum class DataType {
  UNINITIALISED = 0,

  VOID,
  U8,
  S8,
};
std::ostream& operator<<(std::ostream& os, const DataType& type);

unsigned int bytesOf(DataType type);

#endif //COMPILER_VISUALIZATION_TYPES_H
