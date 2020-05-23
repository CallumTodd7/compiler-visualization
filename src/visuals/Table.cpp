//
// Created by Callum Todd on 2020/05/05.
//

#include "Table.h"
#include "love2dHelper.h"

void Table::init(love::graphics::Graphics* g) {
  float columnWidth = frameSize.x - padding.x * 2;
  rowWidth = columnWidth / columns;
  rowWidths[0] = rowWidth / 2;
  rowWidths[1] = rowWidth / 2;
  rowHeight = font->getHeight() * font->getLineHeight() + cellPadding.y * 2;
}

bool Table::has(std::string key) {
  return std::find_if(rows.begin(), rows.end(), [&key](const Row& x) {
    return x.key == key;
  }) != rows.end();
}

love::Vector2 Table::getPositionOf(std::string key) {
  auto it = std::find_if(rows.begin(), rows.end(), [&key](const Row& x) {
    return x.key == key;
  });
  if (it != rows.end()) {
    return it->position;
  }
  return {};
}

void Table::add(love::graphics::Graphics* g, const std::string& key, const std::string& value,
                Position vert, bool isHeading) {
  Row row = {
      .isHeading = isHeading,
      .key = key,
      .value = value,
      .txtKey = g->newText(isHeading ? fontHeading : font, buildColoredString(key, g)),
      .txtValue = g->newText(isHeading ? fontHeading : font, buildColoredString(value, g)),
      .position = {},
  };
  switch (vert) {
    case TOP: {
      if (!rows.empty()) {
        auto it = rows.begin() + 1;
        rows.insert(it, row);
      } else {
        rows.push_back(row);
      }
      break;
    }
    case BOTTOM:
      rows.push_back(row);
      break;
  }
}

void Table::set(love::graphics::Graphics* g, const std::string& key, const std::string& value) {
#if 1
  auto it = std::find_if(rows.begin(), rows.end(), [&](const Row& x) { return x.key == key; });
  if (it != rows.end()) {
    it->value = value;
    it->txtValue = g->newText(font, buildColoredString(value, g));
  }
#else
  for (auto& row : rows) {
    if (row.key == key) {
      row.value = value;
      row.txtValue = g->newText(font, buildColoredString(value, g));
      break;
    }
  }
#endif
}

void Table::remove(std::string key) {
  auto it = rows.erase(std::remove_if(rows.begin(), rows.end(), [&key](const Row& x) {
    return x.key == key;
  }), rows.end());
}

void Table::update(double dt) {

}

void Table::draw(love::graphics::Graphics* g, int xScissorOffset) {
  love::Rect scissorWindow = {
      (int) position.x + xScissorOffset, (int) position.y,
      (int) frameSize.x, (int) frameSize.y
  };
  g->setScissor(scissorWindow);

  love::Vector2 tablePosition = position + padding;

  unsigned int effectiveRowCount = rows.size() + (columns > 0 ? (columns - 1) : columns);
  int rowsPerColumn = ceil(effectiveRowCount / columns);

  for (size_t i = 0; i < rowsPerColumn; ++i) {
    love::Vector2 pos = tablePosition;
    pos.y += rowHeight * (float) (i % rowsPerColumn);

    size_t columnInRow = columns;
    if (i * columns >= rows.size()) {
      columnInRow = columns - (i * columns);
    }

    for (size_t col = 0; col < columns; ++col) {
      Row& row = rows[col * rowsPerColumn + i];

      row.position = pos + love::Vector2(rowWidths[0] + rowWidths[0] / 2, rowHeight / 2);

      love::Matrix4 mat;

      mat.translate(pos.x + cellPadding.x, pos.y + cellPadding.y);
      g->rectangle(love::graphics::Graphics::DrawMode::DRAW_LINE,
                   pos.x, pos.y,
                   rowWidths[0], rowHeight);
      row.txtKey->draw(g, mat);

      mat.translate(rowWidths[0], 0);
      g->rectangle(love::graphics::Graphics::DrawMode::DRAW_LINE,
                   pos.x + rowWidths[0], pos.y,
                   rowWidths[1], rowHeight);
      row.txtValue->draw(g, mat);

      pos.x += rowWidth;
    }
  }

  g->setScissor();
}
