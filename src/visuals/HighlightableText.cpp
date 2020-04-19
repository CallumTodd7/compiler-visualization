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
  std::ifstream fileStream(filepath);
  std::string line;
  while (std::getline(fileStream, line)) {
    textStrings.push_back(love::graphics::Font::ColoredString({line + '\n', colour}));
    // `+ "--"` to add extra blank space at end of content width
    float width = (float) font->getWidth(line + "--");
    if (width > contentSize.x) {
      contentSize.x = width;
    }
  }
  // `+ 1` to add extra blank line at end of content height
  contentSize.y = (float) (textStrings.size() + 1) * (font->getHeight() * font->getLineHeight());

  text->set(textStrings);
}

void HighlightableText::update(double dt) {
  highlightRect.update(dt);
  peekHighlightRect.update(dt);
  scrollManager.update(dt);
}

void HighlightableText::draw(Graphics* g) {
  love::Matrix4 mat = g->getTransform();
  auto scrollOffset = getScrollOffset();
  mat.translate(position.x + padding.x + scrollOffset.x, position.y + padding.y + scrollOffset.y);

  if (text) {
    if (showHighlight || showPeekHighlight) {
      g->push(Graphics::StackType::STACK_ALL);
      if (showPeekHighlight) {
        g->setColor(peekHighlightColour);
        auto rect = peekHighlightRect.get();
        g->rectangle(Graphics::DrawMode::DRAW_FILL,
                     position.x + padding.x + scrollOffset.x + rect.x, position.y + padding.y + scrollOffset.y + rect.y,
                     rect.z, rect.w);
      }
      if (showHighlight) {
        g->setColor(highlightColour);
        auto rect = highlightRect.get();
        g->rectangle(Graphics::DrawMode::DRAW_FILL,
                     position.x + padding.x + scrollOffset.x + rect.x, position.y + padding.y + scrollOffset.y + rect.y,
                     rect.z, rect.w);
      }
      g->pop();
    }

    g->setScissor({
                      (int) position.x, (int) position.y,
                      (int) frameSize.x, (int) frameSize.y
                  });
    text->draw(g, mat);
    g->setScissor();
  }
}

void HighlightableText::highlightPeek(int startLine, int startPos, int endLine, int endPos, double prewait) {
  try {
    std::string str = textStrings.at(startLine - 1).str;
    auto lastTextPos = str.find_first_of("\r\n");
    if (lastTextPos != std::string::npos) {
      str.insert(lastTextPos, "-");
    }

    float x = (float) font->getWidth(str.substr(0, startPos - 1));
    float width = (float) font->getWidth(str.substr(startPos - 1, endPos - startPos + 1));

    love::Vector4 a = {x, font->getHeight() * (float) (startLine - 1), 0, font->getHeight()};
    love::Vector4 b = {x, font->getHeight() * (float) (startLine - 1), width, font->getHeight()};

    bool hasStartPositionMoved = pastStartLinePeekHighlight != startLine || pastStartPosPeekHighlight != startPos;
    peekHighlightRect
        .startAt(a, !hasStartPositionMoved)
        .wait(prewait)
        .goTo(b, 0.5)
        .wait(0.5)
        .finish();
    lastFocusRect = {b.x + b.z, b.y + font->getHeight() * 2};

    showPeekHighlight = true;
    showHighlight = false;
  } catch (std::out_of_range&) {}

  pastStartLinePeekHighlight = startLine;
  pastStartPosPeekHighlight = startPos;
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
    lastFocusRect = {b.x + b.z, b.y + font->getHeight() * 2};

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
      .callback([&] {
        showPeekHighlight = false;
      })
      .finish();
}

love::Vector2 HighlightableText::getScrollOffset() {
  return scrollManager.getOffset(frameSize - padding, contentSize, lastFocusRect);
}
