//
// Created by Callum Todd on 2020/04/14.
//

#ifndef COMPILER_VISUALIZATION_VISUALMAIN_H
#define COMPILER_VISUALIZATION_VISUALMAIN_H

#include <modules/timer/Timer.h>
#include "../../external/love-11.3/src/modules/graphics/opengl/Graphics.h"
#include "../../external/love-11.3/src/modules/font/freetype/Font.h"
#include "../../external/love-11.3/src/modules/graphics/Text.h"
#include "../../external/love-11.3/src/modules/window/sdl/Window.h"
#include "../../external/love-11.3/src/common/Optional.h"
#include "../../external/love-11.3/src/common/delay.h"

#include "../ThreadSync.h"

#include "HighlightableText.h"
#include "Checklist.h"
#include "TokenStream.h"
#include "Tree.h"
#include "Table.h"

using love::graphics::opengl::Shader;
using love::graphics::ShaderStage;
using love::graphics::opengl::Graphics;
using love::graphics::Text;
using love::window::sdl::Window;
using love::timer::Timer;
using love::Optional;
using love::OptionalInt;
using love::OptionalDouble;
using love::Matrix4;

class VisualMain {
private:
  ThreadSync* threadSync;

  Timer* timer;
  Graphics* g;
  Window* window;
  love::font::freetype::Font* font;

  love::graphics::Font* fontVeraRegular19 = nullptr;
  love::graphics::Font* fontSourceCodeProRegular20 = nullptr;

  bool shouldSkipToNextSection = false;

  Data::Mode state = Data::Mode::START;
  Tween<float> horizontalOffset = Tween<float>(0);
  bool hasLexerSupport = false;
  bool hasParserSupport = false;

  float titleHeight = 0;
  Text* txtTitle = nullptr;
  Text* txtTimeText = nullptr;

  bool showError = false;
  Text* txtError = nullptr;

  HighlightableText sourceCode;

  Checklist lexerChecklist;
  bool lexerCurrentIsPeek = false;
  bool shouldScissorLexerCurrent = true;
  bool shouldShowAllChecklistHighlighting = true;
  Tween<Vector2> txtLexerCurrentPos;
  bool isLexerCurrentVisible = false;
  Text* txtLexerCurrent = nullptr;

  TokenStream tokenStream;

  Tree tree;

  Table tblRegisters;
  Table tblLocations;
  Table tblConstants;
  std::vector<std::pair<love::Vector2, love::Vector2>> arrows;
  HighlightableText instructionCode;

public:
  VisualMain(ThreadSync* threadSync, Timer* timer);
  ~VisualMain();

  void init();
  void update(double dt);
  void draw();

  Graphics* getGraphics() {
    return g;
  }

  void skipForwardOneStep();
  void skipToNextSection();
  void setTimeText(const std::string& timeStr);
private:
  void requestNextData();

  void setupNewMode(const Data& data);
  void handleLexerData(const Data& data);
  void handleParserData(const Data& data);
  void handleCodeGenData(const Data& data);

};


#endif //COMPILER_VISUALIZATION_VISUALMAIN_H
