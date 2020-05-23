//
// Created by Callum Todd on 2020/04/18.
//

#include "TokenStream.h"
#include "love2dHelper.h"
#include <math.h>

using love::graphics::Graphics;

float TokenStream::tokenWidth = 250;
float TokenStream::tokenPadding = 3;

VisualToken::VisualToken(const std::string& tokenType, const std::string& value,
                         love::graphics::Font* tokenTypeFont, love::graphics::Font* valueFont) {
  auto* g = love::Module::getInstance<love::graphics::Graphics>(love::Module::ModuleType::M_GRAPHICS);

  if (!tokenTypeFont) {
    tokenTypeFont = g->getFont();
  }
  if (!valueFont) {
    valueFont = g->getFont();
  }

  txtType = g->newText(tokenTypeFont, buildColoredString(tokenType, g));
  txtValue = g->newText(valueFont, buildColoredString(value, g));
}

VisualToken::~VisualToken() {
  txtType->release();
  txtValue->release();
}

void VisualToken::draw(love::graphics::Graphics* g,
                       const love::Vector2& pos,
                       int xScissorOffset,
                       love::Colorf lineColour) {
  float tokenHeight = (g->getFont()->getHeight() + TokenStream::tokenPadding) * 2;

  auto origCol = g->getColor();
  g->setColor(g->getBackgroundColor());
  g->rectangle(love::graphics::Graphics::DrawMode::DRAW_FILL,
               pos.x, pos.y,
               TokenStream::tokenWidth, tokenHeight);
  g->setColor(lineColour);
  g->rectangle(love::graphics::Graphics::DrawMode::DRAW_LINE,
               pos.x, pos.y,
               TokenStream::tokenWidth, tokenHeight);
  g->setColor(origCol);

  g->intersectScissor({
                          (int) pos.x + xScissorOffset, (int) pos.y,
                          (int) TokenStream::tokenWidth, (int) tokenHeight
                      });
  love::Matrix4 mat;
  mat.translate(pos.x + TokenStream::tokenPadding, pos.y + TokenStream::tokenPadding);
  txtType->draw(g, mat);
  mat.translate(0, g->getFont()->getHeight());
  txtValue->draw(g, mat);
  g->setScissor();
}

void TokenStream::add(const love::Vector2& startPosition, const std::string& tokenType, const std::string& value) {
  love::Vector2 endPosition = getNextPosition();

  auto tokenInFlight = new VisualToken(tokenType, value, tokenTypeFont, valueFont);
  tokens.push_back(tokenInFlight);

  auto scrollOffset = getScrollOffset();

  tokenInFlightPosition
      .startAt(startPosition, false)
      .wait(0.5)
      .goTo(scrollOffset + position + padding + endPosition, 1.5)
      .finish();
}

void TokenStream::pop(int newHideTokenCount) {
  if (newHideTokenCount < 0) {
    newHideTokenCount = (int) hideTokenCount.get() + 1;
  }

  hideTokenCount
      .startAtCurrent(0.0)
      .goTo(newHideTokenCount, 1.0)
      .wait(0.5)
      .finish();
}

void TokenStream::update(double dt) {
  tokenInFlightPosition.update(dt);
  verticalFocusPoint.update(dt);
  hideTokenCount.update(dt);
  scrollManager.update(dt);
}

void TokenStream::draw(love::graphics::Graphics* g, int xScissorOffset) {
  auto scrollOffset = getScrollOffset();

  int startIndexLow = floor(hideTokenCount.get());
  int startIndexHigh = ceil(hideTokenCount.get());
  for (size_t i = startIndexLow > 0 ? startIndexLow - 1 : 0; i < tokens.size(); ++i) {
    if (i == tokens.size() - 1 && tokenInFlightPosition.isActive()) {
      g->setScissor();
      tokens[0]->draw(g, tokenInFlightPosition.get(), xScissorOffset, g->getColor());
    } else {
      g->setScissor({
                        (int) position.x + xScissorOffset, (int) position.y,
                        (int) frameSize.x, (int) frameSize.y
                    });
      auto lineColour = g->getColor();
      if (highlightEnabled && i == startIndexHigh) {
        lineColour = {1.0, 1.0, 0.0, 1.0};
      }
      tokens[i]->draw(g, scrollOffset + position + padding + getPositionOf(i), xScissorOffset, lineColour);
    }
  }
  g->setScissor();
}

bool TokenStream::hasActiveAnimations() {
  return tokenInFlightPosition.isActive()
      || verticalFocusPoint.isActive()
      || hideTokenCount.isActive();
}

love::Vector2 TokenStream::getScrollOffset() {
  love::Vector2 hiddenOffset = {
      0,
      -getPositionOf(1).y * hideTokenCount.get()
  };

  auto contentSize = getPositionOf(tokens.size() + 1);
  love::Vector2 focusPoint = {
      0,
      contentSize.y * verticalFocusPoint.get()
  };
  return hiddenOffset + scrollManager.getOffset(frameSize, contentSize, focusPoint);
}

