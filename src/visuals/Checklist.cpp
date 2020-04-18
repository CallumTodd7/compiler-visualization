//
// Created by Callum Todd on 2020/04/15.
//

#include "Checklist.h"

int Checklist::add(Graphics* g, const std::string& text, int parentId) {
  std::vector<love::graphics::Font::ColoredString> strings;
  strings.push_back({text, g->getColor()});
  love::graphics::Text* textObj = g->newText(g->getFont(), strings);

  unsigned char indent = 0;
  if (parentId >= 0 && parentId < items.size()) {
    items[parentId].hasChildren = true;
    indent = items[parentId].indent + 1;
    if (indent > maxIndent) {
      maxIndent = indent;
    }
  }

  Vector2 size = {
      (float) g->getFont()->getWidth(text),
      g->getFont()->getHeight(),
  };

  if (size.x > maxSize.x) {
    maxSize.x = size.x;
  }
  if (size.y > maxSize.y) {
    maxSize.y = size.y;
  }

  ChecklistItem item {
      .text = textObj,
      .size = size,
      .parent = parentId,
      .indent = indent,
      .state = ChecklistItemState::NORMAL
  };

  int id = items.size();
  items.push_back(item);
  updatePositions();
  return id;
}

void Checklist::updatePositions() {
  float indentSize = 10;

  float maxTextWidth = maxSize.x + ((float)maxIndent * indentSize);

  float yPos = 0;

  for (size_t i = 0; i < items.size(); ++i) {
    auto& item = items[i];

    float indentValue = item.indent ? (indentSize * (float)item.indent) : 0;

    if (i > 0) {
      float paddingAmount = padding / (float)(item.indent ? 2 : 1);
      yPos += item.size.y + paddingAmount;
    }

    float alignmentOffset = 0;
    if (alignment == Alignment::RIGHT) {
      alignmentOffset = maxTextWidth - item.size.x;
      // Invert indent
      indentValue = -indentValue;
    } else if (alignment == Alignment::CENTER) {
      alignmentOffset = (maxTextWidth - item.size.x) / 2;
      // No point indenting with center alignment
      indentValue = 0;
    }

    float textXOffset = alignmentOffset + indentValue;

    if (i == cursor) {
      float cursorXOffset = 0;
      if (alignment == Alignment::RIGHT) {
        cursorXOffset = alignmentOffset + item.size.x + cursorPadding;
      }
      cursorPosition = {cursorXOffset, yPos + (item.size.y / 2)};

      if (enableCursor) {
        float height = cursorWidth / 4;
        // Cursor arrow
        cursorRect = {cursorPosition.x, cursorPosition.y - (height / 2),
                      cursorWidth, height};
      }
    }
    if (cursorWidth > 0 && alignment != Alignment::RIGHT) {
      textXOffset += cursorWidth + cursorPadding;
    }

    item._position = {textXOffset, yPos};
  }
}

void Checklist::draw(Graphics* g, const Vector2& position) {
  for (size_t i = 0; i < items.size(); ++i) {
    auto item = items[i];

    if (i == cursor && enableCursor) {
      g->rectangle(Graphics::DrawMode::DRAW_FILL,
                   position.x + cursorRect.x, position.y + cursorRect.y,
                   cursorRect.z, cursorRect.w);
    }

    auto origCol = g->getColor();

    if (highlightUntilYPos < 0 || item._position.y < highlightUntilYPos) {
    switch (item.state) {
      case NORMAL: break;
      case REJECTED:
        g->setColor(love::Colorf(1, 0, 0, 1));
        break;
      case PROGRESS:
        g->setColor(love::Colorf(187 / 255.0f, 186 / 255.0f, 38 / 255.f, 1));
//        g->rectangle(Graphics::DrawMode::DRAW_FILL, position.x + textXOffset, yPos, item.size.x, item.size.y);
        g->setColor(origCol);
        break;
      case ACCEPTED:
        g->setColor(love::Colorf(0, 1, 0, 1));
        break;
      case IGNORED:
        g->setColor(love::Colorf(0.5, 0.5, 0.5, 1));
        break;
    }
    }
    love::Matrix4 textMat;
    textMat.translate(position.x + item._position.x, position.y + item._position.y);
    item.text->draw(g, textMat);

    g->setColor(origCol);
  }
}

void Checklist::next() {
  if (cursor + 1 < items.size()) {
    cursor++;

    updatePositions();
  }
}

void Checklist::accept(unsigned int index, bool isFinal) {
  // Mark prior as rejected
  for (size_t i = 0; i < index; ++i) {
    items[i].state = ChecklistItemState::REJECTED;
  }

  // Mark as accepted

  if (isFinal) {
    items[index].state = ChecklistItemState::ACCEPTED;
  } else {
    items[index].state = ChecklistItemState::PROGRESS;
  }
  cursor = index;

  // Mark rest as ignored

  // While not marking my children...
  int indentOfChildrenToSkip = -1; // -1 for no children
  if (items[index].hasChildren) {
    indentOfChildrenToSkip = items[index].indent + 1;
  }
  // ...mark the rest as ignored.
  for (size_t i = index + 1; i < items.size(); ++i) {
    // Now out of children; reset
    if (indentOfChildrenToSkip > items[i].indent) {
      indentOfChildrenToSkip = -1;
    }

    // If child to skip, skip
    if (indentOfChildrenToSkip >= 0) {
      continue;
    }

    // Mark as ignored
    items[i].state = ChecklistItemState::IGNORED;
  }

  updatePositions();
}

void Checklist::reset() {
  for (auto& item : items) {
    item.state = ChecklistItemState::NORMAL;
  }
  cursor = 0;

  updatePositions();
}

Vector2 Checklist::getCursorPosition() {
  return cursorPosition;
}
