//
// Created by Callum Todd on 2020/02/14.
//

#include <iostream>
#include <functional>
#include "ThreadSync.h"
#include <modules/timer/Timer.h>
#include "compiler/Lexer.h"
#include "compiler/Parser.h"
#include "compiler/Generator.h"
#include "visuals/VisualMain.h"
#include "Data.h"

struct CliOptions {
  char* sourceFilepath = nullptr;
  char* destFilepath = nullptr;
  bool hasUI = true;
};

static CliOptions cliOptions;

int compileWorker(const std::function<void(const Data&)>& ready) {
  printf("Source: %s\nDestination: %s\n\n", cliOptions.sourceFilepath, cliOptions.destFilepath);

  ready({
    .type =  Data::Type::MODE_CHANGE,
    .mode = Data::Mode::LEXER,
    .filepath = std::string(cliOptions.sourceFilepath),
  });

  Lexer lexer(ready, std::string(cliOptions.sourceFilepath));

  std::vector<Token> tokenStream;
  try {
    tokenStream = lexer.getTokenStream();
  } catch (ThreadTerminateException&) {
    throw;
  } catch (std::exception& ex) {
    std::cout << "Lexer threw an exception: " << ex.what() << std::endl;
    ready({
              .mode = Data::Mode::ERROR,
              .type = Data::Type::MODE_CHANGE,
              .string = "An error occurred in the compiler",
          });
    return 1;
  }

  printf("tokenStream length: %lu\n", tokenStream.size());
  for (const Token& token : tokenStream) {
    std::cout << token << std::endl;
  }

  ready({
      .type = Data::Type::MODE_CHANGE,
      .mode = Data::Mode::PARSER,
  });

  Parser parser(ready, tokenStream);
  ASTBlock* root = nullptr;
  try {
    root = parser.parse();
    std::cout << *root << std::endl;
  } catch (ThreadTerminateException&) {
    throw;
  } catch (std::exception& ex) {
    std::cout << "Parser threw an exception: " << ex.what() << std::endl;
    return 1;
  }

  ready({
      .type = Data::Type::MODE_CHANGE,
      .mode = Data::Mode::CODE_GEN,
  });

  try {
    Generator generator(ready, root, cliOptions.destFilepath);
    generator.generate();
  } catch (ThreadTerminateException&) {
    throw;
  } catch (std::exception& ex) {
    std::cout << "Generator threw an exception: " << ex.what() << std::endl;
    return 1;
  }

  ready({
      .type = Data::Type::MODE_CHANGE,
      .mode = Data::Mode::FINISHED,
  });

  return 0;
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

  ThreadSync threadSync(cliOptions.hasUI, true, compileWorker);

  if (cliOptions.hasUI) {
    bool hasCompilerStarted = false;

    auto* timer = new love::timer::Timer();
    love::Module::registerInstance(timer);

    auto visuals = VisualMain(&threadSync, timer);

    visuals.init();

    timer->step();

    double timeModifier = 1.0f;
    bool paused = false;

    // Loop
    SDL_Event evt;
    bool isRunning = true;
    while (isRunning) {
      while (SDL_PollEvent(&evt)) {
        switch (evt.type) {
          case SDL_QUIT:
            isRunning = false;
            break;
          case SDL_KEYUP:
            if (evt.key.keysym.sym == SDLK_ESCAPE) {
              isRunning = false;
            } else if (evt.key.keysym.sym == SDLK_SPACE) {
              if (!hasCompilerStarted) {
                threadSync.runWorker();
                hasCompilerStarted = true;
              } else {
                paused = !paused;
              }
            } else if (evt.key.keysym.sym == SDLK_MINUS) {
              if (timeModifier > 0.05 + 0.009) { // 0.009 to effectively force a round up
                timeModifier /= 1.2;
              }
            } else if (evt.key.keysym.sym == SDLK_EQUALS) {
              timeModifier *= 1.2;
            } else if (evt.key.keysym.sym == SDLK_0) {
              timeModifier = 1;
            } else if (hasCompilerStarted && evt.key.keysym.sym == SDLK_RIGHT) {
              visuals.skipForwardOneStep();
            } else if (hasCompilerStarted && evt.key.keysym.sym == SDLK_DOWN) {
              visuals.skipToNextSection();
            }

            if (evt.key.keysym.sym == SDLK_SPACE
                || evt.key.keysym.sym == SDLK_MINUS
                || evt.key.keysym.sym == SDLK_EQUALS
                || evt.key.keysym.sym == SDLK_0) {
              if (paused) {
                visuals.setTimeText("Paused");
              } else {
                auto ss = std::stringstream()
                    << std::fixed
                    << std::setprecision(timeModifier >= 1 ? 1 : 2);
                ss << timeModifier << "x";
                visuals.setTimeText(ss.str());
              }
            }
            break;
        }
      }

      double dt = timer->step();
      dt *= timeModifier;

      if (!paused) {
        visuals.update(dt);
      }

      auto g = visuals.getGraphics();
      if (g->isActive()) {
        g->origin();
        g->clear(Optional(g->getBackgroundColor()), OptionalInt(0), OptionalDouble(1.0));

        visuals.draw();

        g->present(nullptr);
      }

      timer->sleep(0.001);
    }

    threadSync.terminateThread();
    SDL_Quit();
  } else {
    threadSync.runWorker();
  }

  threadSync.join();

  return threadSync.getWorkerExitCode();
}
