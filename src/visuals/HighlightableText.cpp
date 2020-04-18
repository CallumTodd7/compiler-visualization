//
// Created by Callum Todd on 2020/04/14.
//

#include <fstream>
#include <iostream>
#include "HighlightableText.h"

void HighlightableText::init(Graphics* g) {
  colour = g->getColor();
  font = g->getFont();
  text = g->newText(font);
}

void HighlightableText::load(const std::string& filepath) {
  std::ifstream filestream(filepath);
  std::string line;
  while (std::getline(filestream, line)) {
    textStrings.push_back(love::graphics::Font::ColoredString({line + '\n', colour}));
  }

  text->set(textStrings);
}

void HighlightableText::update(double dt, double mod) {
  highlightRect.update(dt);
  peekHighlightRect.update(dt);
}

void HighlightableText::draw(Graphics* g) {
  love::Matrix4 mat = g->getTransform();
  mat.translate(position.x, position.y);

  if (text) {
    if (showHighlight || showPeekHighlight) {
      g->push(Graphics::StackType::STACK_ALL);
      if (showPeekHighlight) {
        g->setColor(peekHighlightColour);
        auto rect = peekHighlightRect.get();
        g->rectangle(Graphics::DrawMode::DRAW_FILL,
                     position.x + rect.x, position.y + rect.y,
                     rect.z, rect.w);
      }
      if (showHighlight) {
        g->setColor(highlightColour);
        auto rect = highlightRect.get();
        g->rectangle(Graphics::DrawMode::DRAW_FILL,
                     position.x + rect.x, position.y + rect.y,
                     rect.z, rect.w);
      }
      g->pop();
    }
    text->draw(g, mat);
  }
}

void HighlightableText::highlightPeek(int startLine, int startPos, int endLine, int endPos) {
  try {
    const std::string& str = textStrings.at(startLine - 1).str;

    float x1 = (float) font->getWidth(str.substr(0, startPos - 1));
    float x2 = (float) font->getWidth(str.substr(startPos - 1, endPos - startPos + 1));

    love::Vector4 a = {x1, font->getHeight() * (float) (startLine - 1), 0, font->getHeight()};
    love::Vector4 b = {x1, font->getHeight() * (float) (startLine - 1), x2, font->getHeight()};

//    peekHighlightRect
//        .startAt(a)
//        .goTo(b, 0.5)
//        .wait(0.5)
//        .goTo(a, 0.5)
//        .finish();
    peekHighlightRect
        .startAt(a)
        .goTo(b, 0.5)
        .finish();

    showPeekHighlight = true;
    showHighlight = false;
  } catch (std::out_of_range&) {}
}

void HighlightableText::highlight(int startLine, int startPos, int endLine, int endPos) {
  try {
    const std::string& str = textStrings.at(startLine - 1).str;

    float x = (float) font->getWidth(str.substr(0, startPos - 1));
    float width = (float) font->getWidth(str.substr(startPos - 1, endPos - startPos + 1));

    love::Vector4 a = {x, font->getHeight() * (float) (startLine - 1), 0, font->getHeight()};
    love::Vector4 b = {x, font->getHeight() * (float) (startLine - 1), width, font->getHeight()};

    bool hasStartPositionMoved = pastStartLineHighlight != startLine || pastStartPosHighlight != startPos;
    highlightRect
        .startAt(a, !hasStartPositionMoved)
        .goTo(b, 0.5)
        .wait(0.5)
        .finish();

    showHighlight = true;
  } catch (std::out_of_range&) {}

  pastStartLineHighlight = startLine;
  pastStartPosHighlight = startPos;
}

void HighlightableText::unhighlightPeek() {
  auto rect = peekHighlightRect.get();
  rect.z = 0;
  peekHighlightRect
      .startAtCurrent({})
      .goTo(rect, 0.5)
      .finish();
}
