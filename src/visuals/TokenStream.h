//
// Created by Callum Todd on 2020/04/18.
//

#ifndef COMPILER_VISUALIZATION_TOKENSTREAM_H
#define COMPILER_VISUALIZATION_TOKENSTREAM_H

#include <utility>
#include <vector>
#include <string>
#include <common/Vector.h>
#include <modules/graphics/Graphics.h>
#include "Tween.h"

class VisualToken {
private:
  love::graphics::Text* txtType;
  love::graphics::Text* txtValue;

public:
  love::Vector2 position;

public:
  VisualToken(const std::string& tokenType, const std::string& value, const love::Vector2& position,
              love::graphics::Font* tokenTypeFont = nullptr, love::graphics::Font* valueFont = nullptr);
  ~VisualToken();

  void draw(love::graphics::Graphics* g, const love::Vector2& pos);
};

class TokenStream {
private:
  std::vector<VisualToken*> tokens;

  VisualToken* tokenInFlight;
  Tween<love::Vector2> tokenInFlightPosition;

public:
  static float tokenWidth;
  static float tokenPadding;

  love::Vector2 position = {0, 0};
  love::Vector2 padding = {0, 0};

  love::graphics::Font* tokenTypeFont = nullptr;
  love::graphics::Font* valueFont = nullptr;

public:
  ~TokenStream();

  void add(const love::Vector2& startPosition, const std::string& tokenType, const std::string& value = "");

  void update(double dt);
  void draw(love::graphics::Graphics* g);

  bool hasActiveAnimations();

private:
  love::Vector2 getNextPosition() {
    return {0, (50.0f + 10.0f) * tokens.size()};
  }

};


#endif //COMPILER_VISUALIZATION_TOKENSTREAM_H