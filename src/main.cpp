//
// Created by Callum Todd on 2020/02/14.
//

#include <iostream>
#include <thread>
#include "Lexer.h"
#include "ThreadSync.h"
#include "Parser.h"
#include "Generator.h"

#include "../external/love-11.3/src/modules/graphics/opengl/Graphics.h"
#include "../external/love-11.3/src/modules/font/freetype/Font.h"
#include "../external/love-11.3/src/modules/graphics/Text.h"
#include "../external/love-11.3/src/modules/window/sdl/Window.h"
#include "../external/love-11.3/src/common/Optional.h"
#include "../external/love-11.3/src/common/delay.h"

#include "love2dShaders.h"

using love::graphics::opengl::Shader;
using love::graphics::ShaderStage;
using love::graphics::opengl::Graphics;
using love::graphics::Text;
using love::window::sdl::Window;
using love::Optional;
using love::OptionalInt;
using love::OptionalDouble;
using love::Matrix4;

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

Text* newText(Graphics* graphics, const std::string& text) {
  std::vector<love::graphics::Font::ColoredString> strings;
  strings.push_back(love::graphics::Font::ColoredString({text, graphics->getColor()}));
  return graphics->newText(graphics->getFont(), strings);
}

Text* text1 = nullptr;

void init(Graphics* graphics) {
  text1 = newText(graphics, "Test");
}

void update() {

}

void draw(Graphics* graphics) {
  graphics->rectangle(Graphics::DrawMode::DRAW_LINE, 10, 10, 100, 100);
  Matrix4 mat = graphics->getTransform();
  mat.translate(10, 10);
  text1->draw(graphics, mat);
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
    Graphics::defaultShaderCode[Shader::STANDARD_DEFAULT][Shader::LANGUAGE_GLSL3][0].source[ShaderStage::STAGE_VERTEX] = defaultVertexShaderCode;
    Graphics::defaultShaderCode[Shader::STANDARD_DEFAULT][Shader::LANGUAGE_GLSL3][0].source[ShaderStage::STAGE_PIXEL] = defaultPixelShaderCode;
    Graphics::defaultShaderCode[Shader::STANDARD_VIDEO][Shader::LANGUAGE_GLSL3][0].source[ShaderStage::STAGE_VERTEX] = defaultVertexShaderCode;
    Graphics::defaultShaderCode[Shader::STANDARD_VIDEO][Shader::LANGUAGE_GLSL3][0].source[ShaderStage::STAGE_PIXEL] = videoPixelShaderCode;
    Graphics::defaultShaderCode[Shader::STANDARD_ARRAY][Shader::LANGUAGE_GLSL3][0].source[ShaderStage::STAGE_VERTEX] = defaultVertexShaderCode;
    Graphics::defaultShaderCode[Shader::STANDARD_ARRAY][Shader::LANGUAGE_GLSL3][0].source[ShaderStage::STAGE_PIXEL] = arrayPixelShaderCode;

    auto* font = new love::font::freetype::Font();
    love::Module::registerInstance(font);

    auto* graphics = new Graphics();
    love::Module::registerInstance(graphics);
    graphics->setBackgroundColor(love::Colorf(0, 0, 0, 1));

    auto* window = new Window();
    love::Module::registerInstance(window);
    window->setDisplaySleepEnabled(true);
    window->setGraphics(graphics);

    window->setWindow();
    window->setWindowTitle("Compiler Visualisation");

    graphics->setActive(true);

    window->requestAttention(false);

    init(graphics);

    // Loop
    SDL_Event evt;
    while (true) {
      SDL_WaitEvent(&evt);
      if (evt.type == SDL_QUIT) {// || threadSync->isThreadDone()
        break;
      }

      if (!threadSync->isThreadDone()) {
        threadSync->mainReady([&] {
          std::cout << "Copy data" << std::endl;
        });
        std::cout << "Animating!" << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        std::cout << "Animated!" << std::endl;
      }

      if (graphics->isActive()) {
        graphics->origin();
        graphics->clear(Optional(graphics->getBackgroundColor()), OptionalInt(0), OptionalDouble(1.0));

        draw(graphics);

        graphics->present(nullptr);
      }
    }
  }

  SDL_Quit();

  compilationThread.join();

  delete threadSync;

  return workerExitCode;
}
