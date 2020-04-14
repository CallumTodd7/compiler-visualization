//
// Created by Callum Todd on 2020/04/14.
//

#ifndef COMPILER_VISUALIZATION_LOVE2DHELPER_H
#define COMPILER_VISUALIZATION_LOVE2DHELPER_H

#include "../../external/love-11.3/src/modules/graphics/opengl/Graphics.h"
#include "../../external/love-11.3/src/modules/font/freetype/Font.h"
#include "../../external/love-11.3/src/modules/graphics/Text.h"

using love::graphics::Text;
using love::Colorf;

std::vector<love::graphics::Font::ColoredString> buildColoredString(const std::string& text, Graphics* graphics) {
  std::vector<love::graphics::Font::ColoredString> strings;
  strings.push_back(love::graphics::Font::ColoredString({text, graphics->getColor()}));
  return strings;
}

std::vector<love::graphics::Font::ColoredString> buildColoredString(const std::string& text, Colorf colour) {
  std::vector<love::graphics::Font::ColoredString> strings;
  strings.push_back(love::graphics::Font::ColoredString({text, colour}));
  return strings;
}

Text* buildText(Graphics* graphics, const std::string& text) {
  return graphics->newText(graphics->getFont(), buildColoredString(text, graphics));
}

#endif //COMPILER_VISUALIZATION_LOVE2DHELPER_H
