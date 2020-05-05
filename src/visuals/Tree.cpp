//
// Created by Callum Todd on 2020/05/03.
//

#include "Tree.h"
#include "love2dHelper.h"

std::pair<love::Colorf, const char*> nodeChildGroupColors[] = {
    {love::Colorf(1, 0, 0, 1), "red"},
    {love::Colorf(0, 1, 0, 1), "green"},
    {love::Colorf(0, 0, 1, 1), "blue"},
    {love::Colorf(1, 1, 0, 1), "yellow"},
    {love::Colorf(1, 0, 1, 1), "magenta"},
    {love::Colorf(0, 1, 1, 1), "cyan"},
};
#define NODE_CHILD_GROUP_COLOURS_COUNT 6

#if 0
#define SCISSOR_AT_NODE
#endif

float TreeNode::Tag::getWidth() {
  return (float) (txtTagName->getWidth() + txtSpacer->getWidth() + txtValue->getWidth());
}

float TreeNode::Tag::getHeight() {
  if (txtTagName->getHeight() > txtValue->getHeight()) {
    return (float) txtTagName->getHeight();
  } else {
    return (float) txtValue->getHeight();
  }
}

void TreeNode::Tag::draw(love::graphics::Graphics* g, love::Matrix4 mat) {
  txtTagName->draw(g, mat);
  if (txtValue) {
    mat.translate((float) txtTagName->getWidth(), 0);
    txtSpacer->draw(g, mat);
    mat.translate((float) txtSpacer->getWidth(), 0);
    txtValue->draw(g, mat);
  }
}

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
    if (tag.txtTagName) tag.txtTagName->release();
    if (tag.txtValue) tag.txtValue->release();
  }
}

bool TreeNode::hasTag(const std::string& tagName) {
  return std::find_if(tags.begin(), tags.end(), [tagName](const Tag& tag) {
    return tag.tagName == tagName;
  }) != tags.end();
}

void TreeNode::addTag(love::graphics::Graphics* g, love::graphics::Text* spacer,
                      const std::string& tagName, const std::string& colourName, love::Colorf colour) {
  tags.push_back({
                     tagName,
                     g->newText(font, buildColoredString(tagName, g)),
                     spacer,
                     g->newText(font, buildColoredString(colourName + " children", colour)),
                 });

  auto textWidth = tags.back().getWidth() + nodeInnerPadding * 2;
  if (textWidth > size.x) {
    size.x = textWidth;
  }
  size.y += tags.back().getHeight();
}

void TreeNode::addTag(love::graphics::Graphics* g, love::graphics::Text* spacer,
                      const std::string& tagName, const std::string& value) {
  tags.push_back({
                     tagName,
                     g->newText(font, buildColoredString(tagName, g)),
                     spacer,
                     g->newText(valueFont, buildColoredString(value, g))
                 });

  auto textWidth = tags.back().getWidth() + nodeInnerPadding * 2;
  if (textWidth > size.x) {
    size.x = textWidth;
  }
  size.y += tags.back().getHeight();
}

void TreeNode::setNodeType(love::graphics::Graphics* g, const std::string& newNodeType) {
  bool isFirst = nodeType == nullptr;

  nodeType = g->newText(font, buildColoredString(newNodeType, g));

  auto textWidth = (float) nodeType->getWidth() + nodeInnerPadding * 2;
  if (textWidth > size.x) {
    size.x = textWidth;
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

  pos.x += (collectiveRowSize.x - size.x) / 2;

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
    tag.draw(g, mat);
    mat.translate(0, font->getHeight());
  }

#if 0
  float off = 0;
  for (unsigned int i = 0; i < level + 1; ++i) {
    g->rectangle(love::graphics::Graphics::DRAW_FILL, pos.x + nodePadding + off, pos.y + nodePadding, 5, 5);
    off += 10;
  }
  g->rectangle(love::graphics::Graphics::DRAW_LINE, pos.x, pos.y, collectiveRowSize.x, collectiveRowSize.y);
  if (_root == nullptr) {
    g->rectangle(love::graphics::Graphics::DRAW_LINE, offset.x + position.x, pos.y, _rootMaxContentSize.x, _rootMaxContentSize.y);
  }
#endif

  if (drawChildren) {
    level++;
    for (auto child : _children) {
      auto childNode = child.first;
      auto connectionColour = child.second;
      {
        g->setColor(connectionColour);
        std::vector<love::Vector2> lineVerts = {
            {pos.x + (size.x / 2),                                                    pos.y + size.y},
            {offset.x + childNode->position.x + (childNode->collectiveRowSize.x / 2), offset.y + childNode->position.y},
        };
        g->polyline(&lineVerts[0], lineVerts.size());
        g->setColor(origCol);
      }

      childNode->draw(g, offset, scissorWindow, xScissorOffset, selectedNode, true, level);
    }
  }
}

void TreeNode::_recalculateCollectiveSize() {
  if (_children.empty()) {
    collectiveRowSize = size;
  } else {
    float childrenWidth = 0;
    float maxCollectiveContentHeight = 0;
    float maxChildHeight = 0;

    for (auto child : _children) {
      auto childNode = child.first;
      childNode->_recalculateCollectiveSize();

      childrenWidth += childNode->collectiveRowSize.x;

      if (childNode->size.y > maxChildHeight) {
        maxChildHeight = childNode->size.y;
      }
    }
    for (auto child : _children) {
      auto childNode = child.first;
      childNode->collectiveRowSize.y = maxChildHeight;
    }

    auto numberOfGaps = _children.size() - 1;
    childrenWidth += nodePaddingX * (float) numberOfGaps;

    collectiveRowSize = {childrenWidth > size.x ? childrenWidth : size.x, size.y};
  }
}

void TreeNode::_updatePosition(love::Vector2 pos) {
  // Update `_rootMaxContentSize`
  // Note: added to this func as a @hack. Should be in `_recalculateCollectiveSize`.
  if (!_root) {
    _rootMaxContentSize = size;
  }

  position = pos;

  pos.y += collectiveRowSize.y + nodePaddingY;
  for (auto child : _children) {
    auto childNode = child.first;

    childNode->_updatePosition(pos);
    pos.x += childNode->collectiveRowSize.x + nodePaddingX;
  }

  // Update `_rootMaxContentSize`
  // Note: added to this func as a @hack. Should be in `_recalculateCollectiveSize`.
  if (_root) {
    if ((position.x + size.x) - _root->position.x > _root->_rootMaxContentSize.x) {
      _root->_rootMaxContentSize.x = (position.x + size.x) - _root->position.x;
    }
    if (_children.empty()) {
      if ((position.y + size.y) - _root->position.y > _root->_rootMaxContentSize.y) {
        _root->_rootMaxContentSize.y = (position.y + size.y) - _root->position.y;
      }
    }
  }
}

Tree::~Tree() {
  if (txtSpacer) txtSpacer->release();
}

void Tree::init(love::graphics::Graphics* g) {
  txtSpacer = g->newText(font, buildColoredString(": ", g));
}

void Tree::update(double dt) {
  scrollManager.update(dt);
}

bool Tree::hasActiveAnimations() {
  return false;//focusPoint.isActive();
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
  auto colour = nodeChildGroupColors[selectedNode->_childGroupCount % NODE_CHILD_GROUP_COLOURS_COUNT];
  if (!selectedNode->hasTag(groupLabel)) {
    selectedNode->_childGroupCount++;
    colour = nodeChildGroupColors[selectedNode->_childGroupCount % NODE_CHILD_GROUP_COLOURS_COUNT];
    selectedNode->addTag(g, txtSpacer, groupLabel, colour.second, colour.first);
  }

  // New node
  auto* node = new TreeNode(g, font, valueFont);
  node->_root = rootNode;
  node->_parent = selectedNode;

  // Link node as child
  selectedNode->_children.emplace_back(node, colour.first);

  // Select new node
  selectedNode = node;
}

void Tree::addTagToNode(love::graphics::Graphics* g, const std::string& tagName, const std::string& value) {
  if (selectedNode) {
    selectedNode->addTag(g, txtSpacer, tagName, value);
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

love::Vector2 Tree::getScrollOffset() {
  if (!rootNode || !selectedNode) {
    return {};
  }

  love::Vector4 focusArea = {
      selectedNode->position.x, selectedNode->position.y,
      selectedNode->size.x, selectedNode->size.y
  };
  love::Vector4 framePadding = {30, 30, 30, 80};

  return scrollManager.getOffset(frameSize, rootNode->_rootMaxContentSize, focusArea, framePadding);
}
