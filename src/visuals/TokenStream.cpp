//
// Created by Callum Todd on 2020/04/18.
//

#include <iostream>
#include "TokenStream.h"
#include "love2dHelper.h"

using love::graphics::Graphics;

float TokenStream::tokenWidth = 250;
float TokenStream::tokenPadding = 3;

VisualToken::VisualToken(const std::string& tokenType, const std::string& value, const love::Vector2& position,
                         love::graphics::Font* tokenTypeFont, love::graphics::Font* valueFont)
    : position(position) {
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

void VisualToken::draw(love::graphics::Graphics* g, const love::Vector2& pos) {
  float tokenHeight = (g->getFont()->getHeight() + TokenStream::tokenPadding) * 2;

  auto origCol = g->getColor();
  g->setColor(g->getBackgroundColor());
  g->rectangle(love::graphics::Graphics::DrawMode::DRAW_FILL,
               pos.x, pos.y,
               TokenStream::tokenWidth, tokenHeight);
  g->setColor(origCol);
  g->rectangle(love::graphics::Graphics::DrawMode::DRAW_LINE,
               pos.x, pos.y,
               TokenStream::tokenWidth, tokenHeight);

  auto mat = g->getTransform();
  mat.translate(pos.x + TokenStream::tokenPadding, pos.y + TokenStream::tokenPadding);
  txtType->draw(g, mat);
  mat.translate(0, g->getFont()->getHeight());
  txtValue->draw(g, mat);
}

TokenStream::~TokenStream() {
  for (auto token : tokens) {
    delete token;
  }
}

void TokenStream::add(const love::Vector2& startPosition, const std::string& tokenType, const std::string& value) {
  love::Vector2 endPosition = getNextPosition();

  tokenInFlight = new VisualToken(tokenType, value, endPosition, tokenTypeFont, valueFont);

  tokenInFlightPosition
      .startAt(startPosition, false)
      .wait(0.5)
      .goTo(position + padding + endPosition, 1.5)
      .wait(0.5)
      .callback([&] {
        tokens.push_back(tokenInFlight);
      })
      .finish();
}

void TokenStream::update(double dt) {
  tokenInFlightPosition.update(dt);
}

void TokenStream::draw(love::graphics::Graphics* g) {
  for (auto* token : tokens) {
    token->draw(g, position + padding + token->position);
  }
  if (tokenInFlightPosition.isActive()) {
    tokenInFlight->draw(g, tokenInFlightPosition.get());
  }
}

bool TokenStream::hasActiveAnimations() {
  return tokenInFlightPosition.isActive();
}
