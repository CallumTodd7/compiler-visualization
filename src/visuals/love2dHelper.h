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

std::vector<love::graphics::Font::ColoredString> buildColoredString(const std::string& text, love::graphics::Graphics* graphics);

std::vector<love::graphics::Font::ColoredString> buildColoredString(const std::string& text, Colorf colour);

Text* buildText(love::graphics::Graphics* graphics, const std::string& text);

#endif //COMPILER_VISUALIZATION_LOVE2DHELPER_H
