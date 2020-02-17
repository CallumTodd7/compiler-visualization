//
// Created by Callum Todd on 2020/02/14.
//

#include <iostream>
#include "Lexer.h"

int main(int argc, char* argv[]) {
  // Get arguments
  char* sourceFilepath = nullptr;
  char* destFilepath = nullptr;

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-i") == 0) {
      if (i + 1 < argc) {
        sourceFilepath = argv[++i];
      } else {
        std::cerr << "-i option requires one arguments." << std::endl;
        return 1;
      }
    } else if (strcmp(argv[i], "-o") == 0) {
      if (i + 1 < argc) {
        destFilepath = argv[++i];
      } else {
        std::cerr << "-o option requires one arguments." << std::endl;
        return 1;
      }
    }
  }

  if (sourceFilepath == nullptr || destFilepath == nullptr) {
    std::cerr << "Usage: cv -i <sourceFilepath> -o <destFilepath>" << std::endl;
    return 1;
  }

  printf("Source: %s\nDestination: %s\n\n", sourceFilepath, destFilepath);

  Lexer lexer((std::string(sourceFilepath)));
  std::vector<Token> tokenStream = lexer.getTokenStream();
  printf("tokenStream length: %lu\n", tokenStream.size());
  for (const Token& token : tokenStream) {
    std::cout << token << std::endl;
  }

  return 0;
}
