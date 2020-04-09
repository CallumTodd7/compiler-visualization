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

unsigned int bytesOf(DataType type);

#endif //COMPILER_VISUALIZATION_TYPES_H
