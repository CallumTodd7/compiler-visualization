//
// Created by Callum Todd on 2020/04/15.
//

#ifndef COMPILER_VISUALIZATION_CHECKLIST_H
#define COMPILER_VISUALIZATION_CHECKLIST_H

#import <vector>
#import <string>
#include <modules/graphics/opengl/Graphics.h>
#include <modules/graphics/Text.h>

using love::graphics::opengl::Graphics;
using love::Vector2;

enum ChecklistItemState {
  NORMAL,
  REJECTED,
  PROGRESS,
  ACCEPTED,
  IGNORED,
};

struct ChecklistItem {
  love::graphics::Text* text;
  Vector2 size;
  unsigned char indent;
  int parent;
  bool hasChildren = false;
  ChecklistItemState state;
};

enum Alignment {
  LEFT,
  CENTER,
  RIGHT,
};

class Checklist {
public:
  Alignment alignment = Alignment::LEFT;
  bool enableCursor = true;

private:
  std::vector<ChecklistItem> items;
  unsigned int cursor = 0;

  Vector2 maxSize = Vector2(0, 0);
  unsigned char maxIndent = 0;

  Vector2 drawnCursorPosition;

public:
  int add(Graphics* g, const std::string& text, int parentId = -1);

  void next();
  void accept(unsigned int index, bool isFinal);
  void reset();

  void draw(Graphics* g, const Vector2& position);

  Vector2 getDrawnCursorPosition();

private:

};


#endif //COMPILER_VISUALIZATION_CHECKLIST_H
