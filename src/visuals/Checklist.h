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
  ACCEPTED,
};

struct ChecklistItem {
  love::graphics::Text* text;
  unsigned char indent;
  Vector2 size;
  ChecklistItemState state;
};

class Checklist {
private:
  std::vector<ChecklistItem> items;
  unsigned int cursor = 0;

public:
  Checklist& add(Graphics* g, const std::string& text, unsigned char indent = 0);

  void next();
  void accept(unsigned int index);
  void reset();

  void draw(Graphics* g, const Vector2& position);

private:

};


#endif //COMPILER_VISUALIZATION_CHECKLIST_H
