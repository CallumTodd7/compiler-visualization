//
// Created by Callum Todd on 2020/04/14.
//

#include <iostream>
#include "VisualMain.h"
#include "love2dShaders.h"
#include "love2dHelper.h"

VisualMain::VisualMain(ThreadSync* threadSync, Timer* timer)
    : threadSync(threadSync), timer(timer) {
  Graphics::defaultShaderCode[Shader::STANDARD_DEFAULT][Shader::LANGUAGE_GLSL3][0].source[ShaderStage::STAGE_VERTEX] = defaultVertexShaderCode;
  Graphics::defaultShaderCode[Shader::STANDARD_DEFAULT][Shader::LANGUAGE_GLSL3][0].source[ShaderStage::STAGE_PIXEL] = defaultPixelShaderCode;
  Graphics::defaultShaderCode[Shader::STANDARD_VIDEO][Shader::LANGUAGE_GLSL3][0].source[ShaderStage::STAGE_VERTEX] = defaultVertexShaderCode;
  Graphics::defaultShaderCode[Shader::STANDARD_VIDEO][Shader::LANGUAGE_GLSL3][0].source[ShaderStage::STAGE_PIXEL] = videoPixelShaderCode;
  Graphics::defaultShaderCode[Shader::STANDARD_ARRAY][Shader::LANGUAGE_GLSL3][0].source[ShaderStage::STAGE_VERTEX] = defaultVertexShaderCode;
  Graphics::defaultShaderCode[Shader::STANDARD_ARRAY][Shader::LANGUAGE_GLSL3][0].source[ShaderStage::STAGE_PIXEL] = arrayPixelShaderCode;

  font = new love::font::freetype::Font();
  love::Module::registerInstance(font);

  graphics = new Graphics();
  love::Module::registerInstance(graphics);
  graphics->setBackgroundColor(love::Colorf(0, 0, 0, 1));

  window = new Window();
  love::Module::registerInstance(window);
  window->setDisplaySleepEnabled(true);
  window->setGraphics(graphics);

  window->setWindow();
  window->setWindowTitle("Compiler Visualisation");
}

void VisualMain::requestNextData() {
  threadSync->mainReady([&] {
    std::cout << "Copy data" << std::endl;
  });
}

void VisualMain::init() {
  graphics->setActive(true);
  window->requestAttention(false);

  text1 = buildText(graphics, "Test");
}

bool hasRequestedData = false;
void VisualMain::update(double dt) {
//  std::cout << "dt: " << dt << ", time: " << timer->getTime() << std::endl;
  if (((int) Timer::getTime()) % 2 == 0) {
    if (!hasRequestedData) {
      std::cout << "Req data" << std::endl;
      hasRequestedData = true;
      requestNextData();
    }
  } else {
    hasRequestedData = false;
  }
}

void VisualMain::draw() {
  graphics->rectangle(Graphics::DrawMode::DRAW_LINE, 10, 10, 100, 100);
  Matrix4 mat = graphics->getTransform();
  mat.translate(10, 10);
  text1->draw(graphics, mat);
}
