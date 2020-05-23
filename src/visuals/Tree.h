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
#include "love2dHelper.h"

class TreeNode {
private:
  struct Tag {
    std::string tagName;

    love::graphics::Text* txtTagName;
    love::graphics::Text* txtSpacer;
    love::graphics::Text* txtValue;
    love::Optional<love::Colorf> colour;

    float getWidth();
    float getHeight();

    void draw(love::graphics::Graphics* g, love::Matrix4 mat);
  };

private:
  love::graphics::Text* nodeType = nullptr;

  std::vector<Tag> tags;

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

  bool hidden = false;

  love::Vector2 _rootMaxContentSize;

  TreeNode* _root = nullptr;
  TreeNode* _parent = nullptr;
  std::vector<std::pair<TreeNode*, love::Colorf>> _children;

  int _childGroupCount = -1;

  unsigned long nodeId = 0;

public:
  explicit TreeNode(love::graphics::Graphics* g,
                    love::graphics::Font* font = nullptr, love::graphics::Font* valueFont = nullptr);
  ~TreeNode();

  bool hasTag(const std::string& tagName);
  void addTag(love::graphics::Graphics* g, love::graphics::Text* spacer,
              const std::string& tagName, const std::string& colourName, love::Colorf colour);
  void addTag(love::graphics::Graphics* g, love::graphics::Text* spacer,
              const std::string& tagName, const std::string& value);
  std::string getTagNameByColour(love::Colorf colour);
  bool setNodeType(love::graphics::Graphics* g, const std::string& newNodeType);

  void draw(love::graphics::Graphics* g,
            const love::Vector2& offset,
            love::Rect scissorWindow, int xScissorOffset,
            TreeNode* selectedNode,
            bool drawChildren = true, unsigned int level = 0);

  void _recalculateCollectiveSize();
  void _updatePosition(love::Vector2 pos = {});

  TreeNode* findNode(unsigned long targetNodeId);
};

class Tree {
private:
  TreeNode* rootNode = nullptr;
  TreeNode* selectedNode = nullptr;

  ScrollManager scrollManager;

  love::graphics::Text* txtSpacer = nullptr;

public:
  love::Vector2 position = {0, 0};
  Tween<love::Vector2> frameSize;
  love::Vector2 padding = {0, 0};

  love::graphics::Font* font = nullptr;
  love::graphics::Font* valueFont = nullptr;

  love::Vector2 getSelectedNodePosition();
public:
  ~Tree();

  void init(love::graphics::Graphics* g);

  void addNode(love::graphics::Graphics* g, const std::string& groupLabel);
  void insertNodeBetweenChild(love::graphics::Graphics* g, const std::string& groupLabel, unsigned long childNodeId);
  void addTagToNode(love::graphics::Graphics* g, const std::string& tagName, const std::string& value);
  bool setNodeType(love::graphics::Graphics* g, const std::string& nodeType, unsigned long nodeId);
  void selectParentNode();
  void selectNode(unsigned long nodeId);
  void hideSelected();

  void updateNodes();

  void update(double dt);
  void draw(love::graphics::Graphics* g, int xScissorOffset);

  bool hasActiveAnimations();

private:
  love::Vector2 getScrollOffset();

};


#endif //COMPILER_VISUALIZATION_TREE_H
