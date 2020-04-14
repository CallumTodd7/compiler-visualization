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
  Graphics* graphics;
  Window* window;
  love::font::freetype::Font* font;

  Text* text1 = nullptr;

public:
  VisualMain(ThreadSync* threadSync, Timer* timer);

  void init();
  void update(double dt);
  void draw();

  Graphics* getGraphics() {
    return graphics;
  }

private:
  void requestNextData();


};


#endif //COMPILER_VISUALIZATION_VISUALMAIN_H
