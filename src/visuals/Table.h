//
// Created by Callum Todd on 2020/05/05.
//

#ifndef COMPILER_VISUALIZATION_TABLE_H
#define COMPILER_VISUALIZATION_TABLE_H

#include <string>
#include <vector>
#include <modules/graphics/Font.h>
#include <modules/graphics/Text.h>

struct Row {
  bool isHeading;

  std::string key;
  std::string value;
  love::graphics::Text* txtKey;
  love::graphics::Text* txtValue;

  love::Vector2 position;
};

class Table {
public:
  enum Position {
    TOP,
    BOTTOM,
  };

private:
  std::vector<Row> rows;

  love::Vector2 cellPadding = {5, 5};
  float rowWidth;
  float rowWidths[2];
  float rowHeight;

public:
  love::Vector2 position = {0, 0};
  love::Vector2 frameSize;
  love::Vector2 padding = {0, 0};

  int columns = 1;

  love::graphics::Font* font = nullptr;
  love::graphics::Font* fontHeading = nullptr;

public:
  void init(love::graphics::Graphics* g);

  void update(double dt);
  void draw(love::graphics::Graphics* g, int xScissorOffset);

  bool has(std::string key);
  void add(love::graphics::Graphics* g, const std::string& key, const std::string& value,
           Position vert = BOTTOM, bool isHeading = false);
  void set(love::graphics::Graphics* g, const std::string& key, const std::string& value);
  void remove(std::string key);

  bool hasActiveAnimations() {
    return false;
  }

  love::Vector2 getPositionOf(std::string key);
};


#endif //COMPILER_VISUALIZATION_TABLE_H
