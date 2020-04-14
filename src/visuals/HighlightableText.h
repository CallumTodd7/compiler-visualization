//
// Created by Callum Todd on 2020/04/14.
//

#ifndef COMPILER_VISUALIZATION_HIGHLIGHTABLETEXT_H
#define COMPILER_VISUALIZATION_HIGHLIGHTABLETEXT_H

#import <string>
#include <modules/graphics/opengl/Graphics.h>
#include <modules/graphics/Text.h>
#include <modules/graphics/Font.h>

using love::graphics::opengl::Graphics;

class HighlightableText {
public:
  love::Vector2 position;

private:
  std::vector<love::graphics::Font::ColoredString> textStrings;
  love::graphics::Text* text = nullptr;

  love::graphics::Font* font = nullptr;
  love::Colorf colour;

  love::Vector4 highlightPositions;
  love::Colorf highlightColour = love::Colorf(187 / 255.0f, 186 / 255.0f, 38 / 255.0f, 1);

public:
  void load(const std::string& filepath);

  void init(Graphics* g);
  void update(double dt, double mod);
  void draw(Graphics* g);

  void highlight(int startLine, int startPos, int endLine, int endPos);


};


#endif //COMPILER_VISUALIZATION_HIGHLIGHTABLETEXT_H
