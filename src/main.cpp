//
// Created by Callum Todd on 2020/02/14.
//

#include <iostream>
#include <thread>
#include "Lexer.h"
#include "ThreadSync.h"
#include "Parser.h"
#include "Generator.h"

struct CliOptions {
  char* sourceFilepath = nullptr;
  char* destFilepath = nullptr;
  bool hasUI = true;
};

static CliOptions cliOptions;

ThreadSync* threadSync = nullptr;

int workerExitCode = 0;

void compileWorker() {
  printf("Source: %s\nDestination: %s\n\n", cliOptions.sourceFilepath, cliOptions.destFilepath);

  Lexer lexer((std::string(cliOptions.sourceFilepath)));

  std::vector<Token> tokenStream;
  try {
    tokenStream = lexer.getTokenStream();
  } catch (std::exception&) {
    printf("Lexer threw an exception\n");

    workerExitCode = 1;
    threadSync->threadExit();
    return;
  }
  threadSync->workerReady();

  printf("tokenStream length: %lu\n", tokenStream.size());
  for (const Token& token : tokenStream) {
    std::cout << token << std::endl;
  }
  threadSync->workerReady();

  Parser parser(tokenStream);
  ASTBlock* root = nullptr;
  try {
    root = parser.parse();
    std::cout << *root << std::endl;
  } catch (std::exception&) {
    printf("Parser threw an exception\n");

    workerExitCode = 1;
    threadSync->threadExit();
    return;
  }
  threadSync->workerReady();

  Generator generator(root, cliOptions.destFilepath);
  generator.generate();
  threadSync->workerReady();

  workerExitCode = 0;
  threadSync->threadExit();
}

int main(int argc, char* argv[]) {
  // Get arguments
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-i") == 0) {
      if (i + 1 < argc) {
        cliOptions.sourceFilepath = argv[++i];
      } else {
        std::cerr << "-i option requires one arguments." << std::endl;
        return 1;
      }
    } else if (strcmp(argv[i], "-o") == 0) {
      if (i + 1 < argc) {
        cliOptions.destFilepath = argv[++i];
      } else {
        std::cerr << "-o option requires one arguments." << std::endl;
        return 1;
      }
    } else if (strcmp(argv[i], "--no-ui") == 0) {
      cliOptions.hasUI = false;
    }
  }

  if (cliOptions.sourceFilepath == nullptr || cliOptions.destFilepath == nullptr) {
    std::cerr << "Usage: cv -i <sourceFilepath> -o <destFilepath> [--no-ui]" << std::endl;
    return 1;
  }

  threadSync = new ThreadSync(cliOptions.hasUI);

  std::thread compilationThread(compileWorker);

  if (cliOptions.hasUI) {
    while (!threadSync->isThreadDone()) {
      threadSync->mainReady([&]{
        std::cout << "Copy data" << std::endl;
      });
      std::cout << "Animating!" << std::endl;

      std::this_thread::sleep_for(std::chrono::milliseconds(500));

      std::cout << "Animated!" << std::endl;
    }
  }

  compilationThread.join();

  delete threadSync;

  return workerExitCode;
}
