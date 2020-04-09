//
// Created by Callum Todd on 2020/04/09.
//

#include <exception>
#include <iostream>
#include "Types.h"

unsigned int bytesOf(DataType type) {
//  return 8;
  switch (type) {
    case DataType::U8: return 1;
    case DataType::S8: return 1;
//    case DataType::U16: return 2;
//    case DataType::S16: return 2;
//    case DataType::U32: return 4;
//    case DataType::S32: return 4;
//    case DataType::U64: return 8;
//    case DataType::S64: return 8;
    case DataType::VOID: return 0;
    case DataType::UNINITIALISED: {
      std::cout << "Data type is uninitialised!" << std::endl;
      throw std::exception();
    }
  }
}
