//
// Created by Callum Todd on 2020/04/15.
//

#include "Checklist.h"

Checklist& Checklist::add(Graphics* g, const std::string& str, unsigned char indent) {
  std::vector<love::graphics::Font::ColoredString> strings;
  strings.push_back({str, g->getColor()});
  love::graphics::Text* text = g->newText(g->getFont(), strings);

  ChecklistItem item {
      .text = text,
      .indent = indent,
      .size = {
          (float) g->getFont()->getWidth(str),
          g->getFont()->getHeight(),
      },
      .state = ChecklistItemState::NORMAL
  };

  items.push_back(item);
  return *this;
}

void Checklist::draw(Graphics* g, const Vector2& position) {
  float indentSize = 10;
  float paddingSize = 10;
  float cursorWidth = 30;

  love::Matrix4 mat = g->getTransform();
  mat.translate(position.x, position.y);
  float yPos = position.y;

  for (size_t i = 0; i < items.size(); ++i) {
    auto item = items[i];

    float indentValue = item.indent ? (indentSize * (float)item.indent) : 0;

    if (i > 0) {
      float padding = paddingSize / (float)(item.indent ? 2 : 1);
      mat.translate(0, item.size.y + padding);
      yPos += item.size.y + padding;
    }

    love::Matrix4 textMat = mat;
    textMat.translate(cursorWidth + paddingSize + indentValue, 0);

    if (i == cursor) {
      float height = cursorWidth / 4;
      float offset = (item.size.y - height) / 2;
      g->rectangle(Graphics::DrawMode::DRAW_FILL, position.x, yPos + offset, cursorWidth, height);
    }

    auto origCol = g->getColor();
    if (item.state == ChecklistItemState::ACCEPTED) {
      g->setColor(love::Colorf(187 / 255.0f, 186 / 255.0f, 38 / 255.f, 1));
      g->rectangle(Graphics::DrawMode::DRAW_FILL, position.x + cursorWidth + paddingSize + indentValue, yPos, item.size.x, item.size.y);
      g->setColor(origCol);
    } else if (item.state == ChecklistItemState::REJECTED) {
      g->setColor(love::Colorf(1, 0, 0, 1));
    }
    item.text->draw(g, textMat);

    g->setColor(origCol);
  }
}

void Checklist::next() {
  if (cursor + 1 < items.size()) {
    cursor++;
  }
}

void Checklist::accept(unsigned int index) {
  items[index].state = ChecklistItemState::ACCEPTED;
  cursor = index; // TODO Remove me when cursor is being used properly

  // Reject rest
  for (size_t i = index + 1; i < items.size(); ++i) {
    items[i].state = ChecklistItemState::REJECTED;
  }
}

void Checklist::reset() {
  for (auto& item : items) {
    item.state = ChecklistItemState::NORMAL;
  }
  cursor = 0;
}
