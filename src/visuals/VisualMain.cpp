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

  g = new Graphics();
  love::Module::registerInstance(g);
  g->setBackgroundColor(love::Colorf(0, 0, 0, 1));

  window = new Window();
  love::Module::registerInstance(window);
  window->setDisplaySleepEnabled(true);
  window->setGraphics(g);

  love::window::WindowSettings windowSettings;
  windowSettings.resizable = true;
  window->setWindow(1280, 720, &windowSettings);
  window->setWindowTitle("Compiler Visualisation");
}

void VisualMain::requestNextData() {
  threadSync->mainReady([&](const Data& data) {
    std::cout << "Data - type: " << data.type << ", mode: " << data.mode << std::endl;
    switch (data.type) {
      case Data::NOOP: break;
      case Data::MODE_CHANGE:
        setupNewMode(data.mode);
        break;
    }
  });
}

void VisualMain::init() {
  g->setActive(true);
  window->requestAttention(false);

  txtTitle = g->newText(g->getFont(), buildColoredString("Press [SPACE] to start", g));
}

void VisualMain::setupNewMode(Data::Mode newMode) {
  switch (newMode) {
    case Data::LEXER:
      txtTitle->set(buildColoredString("Lexing", g));
      break;
    case Data::PARSER:
      txtTitle->set(buildColoredString("Parsing", g));
      break;
    case Data::CODE_GEN:
      txtTitle->set(buildColoredString("Code Generation", g));
      break;
    case Data::FINISHED:
      txtTitle->set(buildColoredString("Done!", g));
      break;
  }
}

//bool hasRequestedData = false;
void VisualMain::update(double dt) {
//  if (((int) Timer::getTime()) % 2 == 0) {
//    if (!hasRequestedData) {
//      std::cout << "Req data" << std::endl;
//      hasRequestedData = true;
      requestNextData();
//      return;
//    }
//  } else {
//    hasRequestedData = false;
//  }
}

void VisualMain::draw() {
//  g->rectangle(Graphics::DrawMode::DRAW_LINE, 10, 10, 100, 100);

  Matrix4 mat = g->getTransform();
  mat.translate(10, 10);
  txtTitle->draw(g, mat);
}
