//
// Created by Callum Todd on 2020/05/03.
//

#include "Tree.h"
#include "love2dHelper.h"

love::Colorf nodeChildGroupColors[] = {
    love::Colorf(1, 0, 0, 1),
    love::Colorf(0, 1, 0, 1),
    love::Colorf(0, 0, 1, 1),
    love::Colorf(1, 1, 0, 1),
    love::Colorf(1, 0, 1, 1),
    love::Colorf(0, 1, 1, 1),
};
#define NODE_CHILD_GROUP_COLOURS_COUNT 6

#if 0
#define SCISSOR_AT_NODE
#endif

TreeNode::TreeNode(love::graphics::Graphics* g, love::graphics::Font* font, love::graphics::Font* valueFont) {
  if (font) {
    this->font = font;
  } else {
    this->font = g->getFont();
  }
  if (valueFont) {
    this->valueFont = valueFont;
  } else {
    this->valueFont = g->getFont();
  }
}

TreeNode::~TreeNode() {
  for (auto tag : tags) {
    if (tag.second.first) tag.second.first->release();
    if (tag.second.second) tag.second.second->release();
  }
}

bool TreeNode::hasTag(const std::string& tagName) {
  return tags.count(tagName) > 0;
}

void TreeNode::addTag(love::graphics::Graphics* g, const std::string& tagName, love::Colorf colour) {
  tags[tagName] = {
      g->newText(font, buildColoredString(tagName, colour)),
      nullptr
  };

  auto textWidth = (float) tags[tagName].first->getWidth() + nodeInnerPadding * 2;
  if (textWidth > size.x) {
    size.x = textWidth;
  }
  size.y += font->getHeight();
}

void TreeNode::addTag(love::graphics::Graphics* g, const std::string& tagName, const std::string& value) {
  tags[tagName] = {
      g->newText(font, buildColoredString(tagName, g)),
      g->newText(valueFont, buildColoredString(": " + value, g))
  };

  auto textWidth = (float) (tags[tagName].first->getWidth() + tags[tagName].second->getWidth()) + nodeInnerPadding * 2;
  if (textWidth > size.x) {
    size.x = textWidth;
  }
  size.y += (font->getHeight() > valueFont->getHeight() ? font->getHeight() : valueFont->getHeight());
}

void TreeNode::setNodeType(love::graphics::Graphics* g, const std::string& newNodeType) {
  bool isFirst = nodeType == nullptr;

  nodeTypeStr = newNodeType;
  nodeType = g->newText(font, buildColoredString(newNodeType, g));

  auto textWidth = (float) nodeType->getWidth();
  if (textWidth > size.x) {
    size.x = textWidth + nodeInnerPadding * 2;
  }
  if (isFirst) {
    size.y += font->getHeight() + nodeTypePadding;
  }
}

love::Rect intersectRects(love::Rect a, love::Rect b) {
  int x1 = std::max(a.x, b.x);
  int y1 = std::max(a.y, b.y);
  int x2 = std::min(a.x + a.w, b.x + b.w);
  int y2 = std::min(a.y + a.h, b.y + b.h);
  return {x1, y1, std::max(0, x2 - x1), std::max(0, y2 - y1)};
}

void TreeNode::draw(love::graphics::Graphics* g,
                    const love::Vector2& offset,
                    love::Rect scissorWindow, int xScissorOffset,
                    TreeNode* selectedNode,
                    bool drawChildren, unsigned int level) {
  love::Vector2 pos = offset + position;

  auto origCol = g->getColor();
  g->setColor(g->getBackgroundColor());
  g->rectangle(love::graphics::Graphics::DrawMode::DRAW_FILL,
               pos.x, pos.y,
               size.x, size.y);
  if (selectedNode == this) {
    g->setColor(love::Colorf(0, 1, 0, 1));
  } else {
    g->setColor(origCol);
  }
  g->rectangle(love::graphics::Graphics::DrawMode::DRAW_LINE,
               pos.x, pos.y,
               size.x, size.y);
  g->setColor(origCol);

#ifdef SCISSOR_AT_NODE
  g->setScissor(intersectRects(scissorWindow, {
      (int) pos.x + xScissorOffset, (int) pos.y,
      (int) size.x, (int) size.y
  }));
#endif
  love::Matrix4 mat;
  mat.translate(pos.x + nodeInnerPadding, pos.y + nodeInnerPadding);
  if (nodeType) {
    nodeType->draw(g, mat);
    mat.translate(0, font->getHeight() + nodeTypePadding);
  }
  for (auto tag : tags) {
    tag.second.first->draw(g, mat);
    if (tag.second.second) {
      love::Matrix4 mat2 = mat;
      mat2.translate((float) tag.second.first->getWidth(), 0);
      tag.second.second->draw(g, mat2);
    }
    mat.translate(0, font->getHeight());
  }

#if 0
  float off = 0;
  for (unsigned int i = 0; i < level + 1; ++i) {
    g->rectangle(love::graphics::Graphics::DRAW_FILL, pos.x + nodePadding + off, pos.y + nodePadding, 5, 5);
    off += 10;
  }
  g->rectangle(love::graphics::Graphics::DRAW_LINE, pos.x, pos.y, collectiveRowSize.x, collectiveRowSize.y);
#endif
//  g->rectangle(love::graphics::Graphics::DRAW_LINE, pos.x, pos.y, collectiveContentSize.x, collectiveContentSize.y);

  if (drawChildren) {
    level++;
    for (size_t i = 0; i < _children.size(); i++) {
      auto child = _children[i];
      {
        auto origCol = g->getColor();
        g->setColor(nodeChildGroupColors[i % NODE_CHILD_GROUP_COLOURS_COUNT]);
        std::vector<love::Vector2> lineVerts = {
            {pos.x + (size.x / 2), pos.y + size.y},
            {offset.x + child->position.x + (child->size.x / 2), offset.y + child->position.y},
        };
        g->polyline(&lineVerts[0], lineVerts.size());
        g->setColor(origCol);
      }

      child->draw(g, offset, scissorWindow, xScissorOffset, selectedNode, true, level);
    }
  }
}

void TreeNode::_recalculateCollectiveSize() {
  if (_children.empty()) {
    collectiveRowSize = size;
    collectiveContentSize = size;
  } else {
    float childrenWidth = 0;
    float maxCollectiveContentHeight = 0;
    float maxChildHeight = 0;

    for (auto child : _children) {
      child->_recalculateCollectiveSize();

      childrenWidth += child->collectiveRowSize.x;

      if (child->size.y > maxChildHeight) {
        maxChildHeight = child->size.y;
      }
      if (child->collectiveContentSize.y > maxCollectiveContentHeight) {
        maxCollectiveContentHeight = child->collectiveContentSize.y;
      }
    }
    for (auto child : _children) {
      child->collectiveRowSize.y = maxChildHeight;
    }

    auto numberOfGaps = _children.size() - 1;
    childrenWidth += nodePaddingX * (float) numberOfGaps;

    collectiveRowSize = {childrenWidth > size.x ? childrenWidth : size.x, size.y};
    collectiveContentSize = {
        collectiveRowSize.x,
        maxCollectiveContentHeight + size.y + nodePaddingY
    };
  }
}

void TreeNode::_updatePosition(love::Vector2 pos) {
  position = pos;

  pos.y += collectiveRowSize.y + nodePaddingY;
  for (auto child : _children) {
    child->_updatePosition(pos);
    pos.x += child->collectiveRowSize.x + nodePaddingX;
  }
}

void Tree::update(double dt) {
  focusPoint.update(dt);
  scrollManager.update(dt);
}

bool Tree::hasActiveAnimations() {
  return focusPoint.isActive();
}

void Tree::addNode(love::graphics::Graphics* g, const std::string& groupLabel) {
  if (rootNode == nullptr) {
    // Add root node
    auto* node = new TreeNode(g, font, valueFont);
    rootNode = node;
    selectedNode = node;
    return;
  }

  // Add tag to parent node
  if (!selectedNode->hasTag(groupLabel)) {
    auto colour = nodeChildGroupColors[selectedNode->_childGroupCount % NODE_CHILD_GROUP_COLOURS_COUNT];
    selectedNode->addTag(g, groupLabel, colour);
    selectedNode->_childGroupCount++;
  }

  // New node
  auto* node = new TreeNode(g, font, valueFont);
  node->_parent = selectedNode;

  // Link node as child
  selectedNode->_children.push_back(node);

  // Select new node
  selectedNode = node;
}

void Tree::addTagToNode(love::graphics::Graphics* g, const std::string& tagName, const std::string& value) {
  if (selectedNode) {
    selectedNode->addTag(g, tagName, value);
  }
}

void Tree::setNodeType(love::graphics::Graphics* g, const std::string& nodeType) {
  if (selectedNode) {
    selectedNode->setNodeType(g, nodeType);
  }
}

void Tree::selectParentNode() {
  if (selectedNode && selectedNode->_parent) {
    selectedNode = selectedNode->_parent;
  }
}

void Tree::updateNodes() {
  if (rootNode) {
    rootNode->_recalculateCollectiveSize();
    rootNode->_updatePosition();
    focusPoint.set(selectedNode->position);
  }
}

void Tree::draw(love::graphics::Graphics* g, int xScissorOffset) {
  if (!rootNode) {
    return;
  }

  love::Vector2 scrollOffset = getScrollOffset();

  love::Vector2 centreOffset = {
      0,//(frameSize.x - rootNode->size.x) / 2,
      0
  };

  love::Rect scissorWindow = {
      (int) position.x + xScissorOffset, (int) position.y,
      (int) frameSize.x, (int) frameSize.y
  };
#ifndef SCISSOR_AT_NODE
  g->setScissor(scissorWindow);
#endif

  rootNode->draw(g, position + scrollOffset + centreOffset + padding, scissorWindow, xScissorOffset, selectedNode);

  g->setScissor();
}

love::Vector2 Tree::getContentSize() {
  if (!rootNode) {
    return frameSize;
  }
  return rootNode->collectiveContentSize;
}

love::Vector2 Tree::getScrollOffset() {
  auto contentSize = getContentSize();
  return scrollManager.getOffset(frameSize, contentSize, focusPoint.get());
}
