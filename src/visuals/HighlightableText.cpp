//
// Created by Callum Todd on 2020/04/14.
//

#include <fstream>
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

}

void HighlightableText::draw(Graphics* g) {
  love::Matrix4 mat = g->getTransform();
  mat.translate(position.x, position.y);

  if (text) {
    if (showHighlight || showPeekHighlight) {
      g->push(Graphics::StackType::STACK_ALL);
      if (showPeekHighlight) {
        g->setColor(peekHighlightColour);
        g->rectangle(Graphics::DrawMode::DRAW_FILL,
                     position.x + peekHighlightPositions.x, position.y + peekHighlightPositions.y,
                     peekHighlightPositions.z, peekHighlightPositions.w);
      }
      if (showHighlight) {
        g->setColor(highlightColour);
        g->rectangle(Graphics::DrawMode::DRAW_FILL,
                     position.x + highlightPositions.x, position.y + highlightPositions.y,
                     highlightPositions.z, highlightPositions.w);
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

    peekHighlightPositions = {
        x1, font->getHeight() * (float) (startLine - 1), x2, font->getHeight()
    };
    showPeekHighlight = true;
    showHighlight = false;
  } catch (std::out_of_range&) {}
}

void HighlightableText::highlight(int startLine, int startPos, int endLine, int endPos) {
  try {
    const std::string& str = textStrings.at(startLine - 1).str;

    float x1 = (float) font->getWidth(str.substr(0, startPos - 1));
    float x2 = (float) font->getWidth(str.substr(startPos - 1, endPos - startPos + 1));

    highlightPositions = {
        x1, font->getHeight() * (float) (startLine - 1), x2, font->getHeight()
    };
    showHighlight = true;
  } catch (std::out_of_range&) {}
}
