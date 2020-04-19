//
// Created by Callum Todd on 2020/04/19.
//

#include "love2dHelper.h"

using love::graphics::Graphics;
using love::graphics::Text;
using love::Colorf;

std::vector <love::graphics::Font::ColoredString> buildColoredString(const std::string& text, Graphics* graphics) {
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
