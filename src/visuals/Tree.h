//
// Created by Callum Todd on 2020/05/03.
//

#ifndef COMPILER_VISUALIZATION_TREE_H
#define COMPILER_VISUALIZATION_TREE_H

#include <common/Vector.h>
#include <modules/graphics/Font.h>
#include <modules/graphics/Graphics.h>
#include <modules/graphics/opengl/Graphics.h>
#include "Tween.h"
#include "ScrollManager.h"

class TreeNode {
private:
  std::map<std::string, std::pair<love::graphics::Text*, love::graphics::Text*>> tags;
  std::string nodeTypeStr;
  love::graphics::Text* nodeType = nullptr;

  love::graphics::Font* font;
  love::graphics::Font* valueFont;

  static constexpr float nodeInnerPadding = 10;
  static constexpr float nodeTypePadding = 2;

  static constexpr float nodePaddingX = 10;
  static constexpr float nodePaddingY = 30;

public:
  love::Vector2 position;
  love::Vector2 size = {nodeInnerPadding * 2, nodeInnerPadding * 2};
  love::Vector2 collectiveRowSize;
  love::Vector2 collectiveContentSize;

  TreeNode* _parent = nullptr;
  std::vector<TreeNode*> _children;

  int _childGroupCount = 0;

public:
  explicit TreeNode(love::graphics::Graphics* g,
                    love::graphics::Font* font = nullptr, love::graphics::Font* valueFont = nullptr);
  ~TreeNode();

  bool hasTag(const std::string& tagName);
  void addTag(love::graphics::Graphics* g, const std::string& tagName, love::Colorf colour);
  void addTag(love::graphics::Graphics* g, const std::string& tagName, const std::string& value);
  void setNodeType(love::graphics::Graphics* g, const std::string& newNodeType);

  void draw(love::graphics::Graphics* g,
            const love::Vector2& offset,
            love::Rect scissorWindow, int xScissorOffset,
            TreeNode* selectedNode,
            bool drawChildren = true, unsigned int level = 0);

  void _recalculateCollectiveSize();
  void _updatePosition(love::Vector2 pos = {});
};

class Tree {
private:
  TreeNode* rootNode = nullptr;
  TreeNode* selectedNode = nullptr;

  ScrollManager scrollManager;

public:
  Tween<love::Vector2> focusPoint;

  love::Vector2 position = {0, 0};
  love::Vector2 frameSize = {0, 0};
  love::Vector2 padding = {0, 0};

  love::graphics::Font* font = nullptr;
  love::graphics::Font* valueFont = nullptr;

public:
  void addNode(love::graphics::Graphics* g, const std::string& groupLabel);
  void addTagToNode(love::graphics::Graphics* g, const std::string& tagName, const std::string& value);
  void setNodeType(love::graphics::Graphics* g, const std::string& nodeType);
  void selectParentNode();
  void updateNodes();

  void update(double dt);
  void draw(love::graphics::Graphics* g, int xScissorOffset);

  bool hasActiveAnimations();

private:
  love::Vector2 getContentSize();
  love::Vector2 getScrollOffset();

};


#endif //COMPILER_VISUALIZATION_TREE_H
