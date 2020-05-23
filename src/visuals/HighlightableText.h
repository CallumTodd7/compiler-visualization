//
// Created by Callum Todd on 2020/04/14.
//

#ifndef COMPILER_VISUALIZATION_HIGHLIGHTABLETEXT_H
#define COMPILER_VISUALIZATION_HIGHLIGHTABLETEXT_H

#import <string>
#import <functional>
#include <modules/graphics/opengl/Graphics.h>
#include <modules/graphics/Text.h>
#include <modules/graphics/Font.h>
#include "Tween.h"
#include "ScrollManager.h"

using love::graphics::opengl::Graphics;

class HighlightableText {
public:
  love::Vector2 position;
  love::Vector2 padding;
  love::Vector2 frameSize;

private:
  std::vector<love::graphics::Font::ColoredString> textStrings;
  unsigned long lineCount = 0;
  love::graphics::Text* text = nullptr;

  love::Vector2 contentSize;

  love::graphics::Font* font = nullptr;
  love::Colorf colour;
  love::Colorf colourComment;

  bool showHighlight = false;
  Tween<love::Vector4> highlightRect;
  love::Colorf highlightColour = love::Colorf(187 / 255.0f, 186 / 255.0f, 38 / 255.0f, 1);
  int pastStartLineHighlight = -1;
  int pastStartPosHighlight = -1;
  int pastStartLinePeekHighlight = -1;
  int pastStartPosPeekHighlight = -1;

  bool showPeekHighlight = false;
  Tween<love::Vector4> peekHighlightRect;
  love::Colorf peekHighlightColour = love::Colorf(187 / 255.0f, 38 / 255.0f, 186 / 255.0f, 1);

  ScrollManager scrollManager;
  love::Vector2 lastFocusRect;

public:
  void load(const std::string& filepath);
  void add(const std::string& str);

  void init(Graphics* g);
  void update(double dt);
  void draw(Graphics* g, int xScissorOffset);

  void highlight(int startLine, int startPos, int endLine, int endPos);
  void highlightPeek(int startLine, int startPos, int endLine, int endPos, double prewait = 0.0);
  void unhighlightPeek();

  bool hasActiveAnimations() {
    return highlightRect.isActive()
        || peekHighlightRect.isActive();
  }

  love::Vector4 getHighlightRect() {
    return highlightRect.get();
  }
  love::Vector4 getPeekHighlightRect() {
    return peekHighlightRect.get();
  }

private:
  love::Vector2 getScrollOffset();

};


#endif //COMPILER_VISUALIZATION_HIGHLIGHTABLETEXT_H
