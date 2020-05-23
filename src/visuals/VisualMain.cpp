//
// Created by Callum Todd on 2020/04/14.
//

#include <iostream>
#include <sstream>
#include "VisualMain.h"
#include "love2dShaders.h"
#include "love2dHelper.h"

// Font
#include "SourceCodePro-regular.ttf.h"
#include "font/Vera.ttf.h"

class VeraFontData : public love::Data {
public:
  Data* clone() const override { return new VeraFontData(); }

  void* getData() const override { return Vera_ttf; }

  size_t getSize() const override { return sizeof(Vera_ttf); }
};

class SourceCodeProFontData : public love::Data {
public:
  Data* clone() const override { return new SourceCodeProFontData(); }

  void* getData() const override { return SourceCodePro_Regular_ttf; }

  size_t getSize() const override { return SourceCodePro_Regular_ttf_len; }
};

using love::font::TrueTypeRasterizer;

VisualMain::VisualMain(ThreadSync* threadSync, Timer* timer)
    : threadSync(threadSync), timer(timer) {
  auto shaderStandard = &Graphics::defaultShaderCode[Shader::STANDARD_DEFAULT][Shader::LANGUAGE_GLSL3][0];
  shaderStandard->source[ShaderStage::STAGE_VERTEX] = defaultVertexShaderCode;
  shaderStandard->source[ShaderStage::STAGE_PIXEL] = defaultPixelShaderCode;
  auto shaderVideo = &Graphics::defaultShaderCode[Shader::STANDARD_VIDEO][Shader::LANGUAGE_GLSL3][0];
  shaderVideo->source[ShaderStage::STAGE_VERTEX] = defaultVertexShaderCode;
  shaderVideo->source[ShaderStage::STAGE_PIXEL] = videoPixelShaderCode;
  auto shaderArray = &Graphics::defaultShaderCode[Shader::STANDARD_ARRAY][Shader::LANGUAGE_GLSL3][0];
  shaderArray->source[ShaderStage::STAGE_VERTEX] = defaultVertexShaderCode;
  shaderArray->source[ShaderStage::STAGE_PIXEL] = arrayPixelShaderCode;

  font = new love::font::freetype::Font();
  love::Module::registerInstance(font);

  g = new Graphics();
  love::Module::registerInstance(g);
  g->setBackgroundColor(love::Colorf(0, 0, 0, 1));

  window = new Window();
  love::Module::registerInstance(window);
  window->setDisplaySleepEnabled(true);
  window->setGraphics(g);

  love::window::WindowSettings windowSettings;
  windowSettings.resizable = true;
  windowSettings.vsync = false; // TODO remove or keep?
  window->setWindow(1280, 720, &windowSettings);
  window->setWindowTitle("Compiler Visualisation");
}

VisualMain::~VisualMain() {
  if (timer) timer->release();
  if (g) g->release();
  if (window) window->release();
  if (font) font->release();

  if (fontVeraRegular19) fontVeraRegular19->release();
  if (fontSourceCodeProRegular20) fontSourceCodeProRegular20->release();

  if (txtTitle) txtTitle->release();
  if (txtTimeText) txtTimeText->release();
  if (txtLexerCurrent) txtLexerCurrent->release();
}

void VisualMain::requestNextData() {
  do {
    threadSync->getData([&](const Data& data) {
      //    std::cout << "Data - type: " << data.type << ", mode: " << data.mode << std::endl;
      switch (data.type) {
        case Data::Type::NOOP:
          shouldSkipToNextSection = false;
          tweenNoAnimate = false;
          break;
        case Data::Type::MODE_CHANGE:
          shouldSkipToNextSection = false;
          tweenNoAnimate = false;

          setupNewMode(data);
          break;
        case Data::Type::SPECIFIC:
          switch (data.mode) {
            case Data::START:
              break;
            case Data::LEXER:
              handleLexerData(data);
              break;
            case Data::PARSER:
              handleParserData(data);
              break;
            case Data::CODE_GEN:
              handleCodeGenData(data);
              break;
            case Data::FINISHED:
              // Finished here
              break;
            case Data::ERROR:
              break;
          }
          break;
      }
    });
  } while (threadSync->isDataQueued() && shouldSkipToNextSection);

  tweenNoAnimate = false;
}

template<typename FontData>
love::graphics::Font* genFont(int size, TrueTypeRasterizer::Hinting hinting) {
  auto g = love::Module::getInstance<Graphics>(love::Module::ModuleType::M_GRAPHICS);
  auto font = love::Module::getInstance<love::font::Font>(love::Module::ModuleType::M_FONT);

  auto data = new FontData();
  auto rasterizer = font->newTrueTypeRasterizer(data, size, hinting);
  auto newFont = g->newFont(rasterizer, g->getDefaultFilter());
  rasterizer->release();
  data->release();

  return newFont;
}

void VisualMain::init() {
  g->setActive(true);
  window->requestAttention(false);

  fontVeraRegular19 = genFont<VeraFontData>(19, TrueTypeRasterizer::Hinting::HINTING_NORMAL);
  fontSourceCodeProRegular20 = genFont<SourceCodeProFontData>(20, TrueTypeRasterizer::Hinting::HINTING_NORMAL);
  g->setFont(fontSourceCodeProRegular20);

  titleHeight = fontVeraRegular19->getHeight() + 20;

  txtTitle = g->newText(fontVeraRegular19, buildColoredString("Press [SPACE] to start", g));
  txtTimeText = g->newText(fontVeraRegular19);
  txtLexerCurrent = g->newText(g->getFont());

  txtError = g->newText(fontVeraRegular19);

  tokenStream.padding = {10, 10};
  tokenStream.frameSize = {
      TokenStream::tokenWidth + tokenStream.padding.x * 2,
      (float) g->getHeight() - titleHeight - tokenStream.padding.y
  };
  tokenStream.position = {(float) g->getWidth() - tokenStream.frameSize.x, titleHeight};
  tokenStream.tokenTypeFont = fontVeraRegular19;
  tokenStream.valueFont = fontSourceCodeProRegular20;

  sourceCode.init(g);
  sourceCode.position = {0, titleHeight};
  sourceCode.padding = {10, 5};
  sourceCode.frameSize = love::Vector2(tokenStream.position.x / 2, (float) g->getHeight()) - sourceCode.position;

  lexerChecklist.position = {tokenStream.position.x / 2, titleHeight + 15};
  lexerChecklist.alignment = Alignment::RIGHT;
  lexerChecklist.enableCursor = false;
  lexerChecklist.cursorPadding = 20;
  auto charWidth = (float) g->getFont()->getWidth('0');
  lexerChecklist.cursorWidth = charWidth;
  lexerChecklist.indentSize = charWidth;

  auto origFont = g->getFont();
  g->setFont(fontVeraRegular19);
  lexerChecklist.add(g, "End of file");
  lexerChecklist.add(g, "Number");
  lexerChecklist.add(g, "String literal");
  auto alphaId = lexerChecklist.add(g, "Alphabetic character");
  lexerChecklist.add(g, "Keyword -", alphaId);
  lexerChecklist.add(g, "Identifier -", alphaId);
  lexerChecklist.add(g, "Operator");
  lexerChecklist.add(g, "Error: Unknown token");
  g->setFont(origFont);

  tree.padding = {0, 10};
  tree.frameSize.set({
      (float) g->getWidth() - tokenStream.frameSize.x,
      (float) g->getHeight() - titleHeight - tree.padding.y
  });
  tree.position = {(float) g->getWidth(), titleHeight};
  tree.font = fontVeraRegular19;
  tree.valueFont = fontSourceCodeProRegular20;
  tree.init(g);

  tblRegisters.position = {(float) g->getWidth() + (float) g->getWidth() / 3, titleHeight};
  tblRegisters.padding = {10, 10};
  tblRegisters.frameSize = {(float) g->getWidth() / 3, ((float) g->getHeight() - sourceCode.position.y) / 3};
  tblRegisters.columns = 2;
  tblRegisters.font = fontVeraRegular19;
  tblRegisters.fontHeading = fontVeraRegular19;
  tblRegisters.init(g);
  tblRegisters.add(g, "Register", "Content", Table::Position::BOTTOM, true);
  tblRegisters.add(g, "RAX", "");
  tblRegisters.add(g, "RBX", "");
  tblRegisters.add(g, "RCX", "");
  tblRegisters.add(g, "RDX", "");
  tblRegisters.add(g, "R8", "");
  tblRegisters.add(g, "R9", "");
  tblRegisters.add(g, "R10", "");
  tblRegisters.add(g, "R11", "");
  tblRegisters.add(g, "RSI", "");
  tblRegisters.add(g, "RDI", "");
  tblRegisters.add(g, "R15", "");

  tblLocations.position = {(float) g->getWidth() + (float) g->getWidth() / 3, titleHeight + tblRegisters.frameSize.y};
  tblLocations.padding = {10, 10};
  tblLocations.frameSize = {(float) g->getWidth() / 3, ((float) g->getHeight() - sourceCode.position.y) / 3};
  tblLocations.font = fontVeraRegular19;
  tblLocations.fontHeading = fontVeraRegular19;
  tblLocations.init(g);
  tblLocations.add(g, "ID", "Location", Table::Position::BOTTOM, true);

  tblConstants.position = {(float) g->getWidth() + (float) g->getWidth() / 3, titleHeight + tblRegisters.frameSize.y + tblLocations.frameSize.y};
  tblConstants.padding = {10, 10};
  tblConstants.frameSize = {(float) g->getWidth() / 3, ((float) g->getHeight() - sourceCode.position.y) / 3};
  tblConstants.font = fontVeraRegular19;
  tblConstants.fontHeading = fontVeraRegular19;
  tblConstants.init(g);
  tblConstants.add(g, "Constant ID", "Data", Table::Position::BOTTOM, true);

  instructionCode.init(g);
  instructionCode.position = {(float) g->getWidth() + (float) g->getWidth() / 3 * 2, titleHeight};
  instructionCode.padding = {10, 5};
  instructionCode.frameSize = love::Vector2((float) g->getWidth() / 3, (float) g->getHeight() - sourceCode.position.y);
}

void VisualMain::setupNewMode(const Data& data) {
  if (data.mode != Data::Mode::ERROR) {
    state = data.mode;
  }

  switch (data.mode) {
    case Data::START:
      break;
    case Data::LEXER:
      txtTitle->set(buildColoredString("Lexing", g));
      sourceCode.load(data.filepath);
      break;
    case Data::PARSER:
      txtTitle->set(buildColoredString("Parsing", g));
      hasLexerSupport = true;
      horizontalOffset
          .startAtCurrent({})
          .goTo(horizontalOffset.get() - tokenStream.position.x, 4)
          .callback([&] {
            hasLexerSupport = false;
          })
          .wait(1.0)
          .finish();
      tokenStream.highlightEnabled = true;
      tokenStream.verticalFocusPoint
          .startAtCurrent(0.0)
          .goTo(0.0, 3)
          .finish();
      break;
    case Data::CODE_GEN:
      txtTitle->set(buildColoredString("Code Generation", g));
      hasParserSupport = true;
      horizontalOffset
          .startAtCurrent({})
          .goTo(horizontalOffset.get() - tokenStream.frameSize.x, 4)
          .callback([&] {
            hasParserSupport = false;
          })
          .wait(1.0)
          .finish();
      tree.frameSize
          .startAtCurrent({})
          .goTo({(float) g->getWidth() / 3, tree.frameSize.get().y}, 4)
          .wait(1.0)
          .finish();
      break;
    case Data::FINISHED:
      txtTitle->set(buildColoredString("Done!", g));
      break;
    case Data::ERROR: {
      if (!showError) {
        txtError->set(buildColoredString(data.string, g));
        txtTitle->set(buildColoredString("Error", g));
        showError = true;
      }
      break;
    }
  }
}

void VisualMain::handleLexerData(const Data& data) {
  if (data.lexerState == Data::LexerState::NEW_TOKEN) {
    txtLexerCurrentPos.set({
                               lexerChecklist.getCursorPosition().x,
                               lexerChecklist.position.y + lexerChecklist.padding
                           });
    isLexerCurrentVisible = true;

    sourceCode.highlightPeek(data.lexerContextStart.lineNumber, data.lexerContextStart.characterPos,
                             data.lexerContextEnd.lineNumber, data.lexerContextEnd.characterPos);
    lexerCurrentIsPeek = true;
  } else if (data.lexerState == Data::LexerState::WORD_UPDATE) {
    sourceCode.highlightPeek(data.lexerContextStart.lineNumber, data.lexerContextStart.characterPos,
                             data.lexerContextEnd.lineNumber, data.lexerContextEnd.characterPos + 1,
                             data.lexerContextStart.characterPos == data.lexerContextEnd.characterPos ? 0.5 : 0.0);
    sourceCode.highlight(data.lexerContextStart.lineNumber, data.lexerContextStart.characterPos,
                         data.lexerContextEnd.lineNumber, data.lexerContextEnd.characterPos);
    lexerCurrentIsPeek = false;
  }

  if (data.peekedChar > 0) {
    txtLexerCurrent->set(buildColoredString(std::string(1, data.peekedChar), g));
    isLexerCurrentVisible = true;
  }

  if (data.lexerState == Data::LexerState::WORD_UPDATE) {
    txtLexerCurrent->set(buildColoredString(data.string, g));
    isLexerCurrentVisible = true;
    if (data.string.size() > 1) {
      shouldScissorLexerCurrent = true;
    }
  }

  if (data.lexerState == Data::LexerState::NEW_TOKEN) {
    lexerChecklist.reset();
    return;
  }

  auto moveToPos = [&](int travelIndexDistance, double wait = 0.0, const std::function<void()>& callback = nullptr) {
    shouldShowAllChecklistHighlighting = false;
    txtLexerCurrentPos
        .startAtCurrent({})
        .goTo(lexerChecklist.getCursorPosition(), 0.5 * travelIndexDistance)
        .callback([&] {
          shouldScissorLexerCurrent = false;
          shouldShowAllChecklistHighlighting = true;
          sourceCode.unhighlightPeek();
        })
        .wait(wait)
        .callback(callback)
        .finish();
  };

  if (data.lexerState == Data::LexerState::END_OF_FILE) {
    lexerChecklist.accept(0, true);
    moveToPos(0);
  } else if (data.lexerState == Data::LexerState::START_NUMBER) {
    lexerChecklist.accept(1, false);
    moveToPos(1);
  } else if (data.lexerState == Data::LexerState::END_NUMBER) {
    lexerChecklist.accept(1, true);
    tokenStream.add(lexerChecklist.getCursorPosition(false), data.tokenType, data.string);
    isLexerCurrentVisible = false;
  } else if (data.lexerState == Data::LexerState::START_STRING) {
    lexerChecklist.accept(2, false);
    moveToPos(2);
  } else if (data.lexerState == Data::LexerState::END_STRING) {
    lexerChecklist.accept(2, true);
    tokenStream.add(lexerChecklist.getCursorPosition(false), data.tokenType, data.string);
    isLexerCurrentVisible = false;
  } else if (data.lexerState == Data::LexerState::START_ALPHA) {
    lexerChecklist.accept(3, false);
    moveToPos(3);
  } else if (data.lexerState == Data::LexerState::END_ALPHA_KEYWORD) {
    lexerChecklist.accept(4, true);
    moveToPos(1, 0.5, [&] {
      tokenStream.add(lexerChecklist.getCursorPosition(false), data.tokenType);
      isLexerCurrentVisible = false;
    });
  } else if (data.lexerState == Data::LexerState::END_ALPHA_IDENT) {
    lexerChecklist.accept(5, true);
    moveToPos(2, 0.5, [&] {
      tokenStream.add(lexerChecklist.getCursorPosition(false), data.tokenType, data.string);
      isLexerCurrentVisible = false;
    });
  } else if (data.lexerState == Data::LexerState::START_OP) {
    lexerChecklist.accept(6, false);
    moveToPos(6);
  } else if (data.lexerState == Data::LexerState::END_OP) {
    lexerChecklist.accept(6, true);
    tokenStream.add(lexerChecklist.getCursorPosition(false), data.tokenType);
    isLexerCurrentVisible = false;
  } else if (data.lexerState == Data::LexerState::UNKNOWN) {
    lexerChecklist.accept(7, true);
    moveToPos(7);
  }
}

int shouldSetHideTokenCount = -1;

void VisualMain::handleParserData(const Data& data) {
  switch (data.parserState) {
    case Data::PARSER_UNINITIALISED:
//      std::cout << "Parser - type: " << data.type << ", state: PARSER_UNINITIALISED" << std::endl;
      break;
    case Data::START_NODE:
//      std::cout << "Parser - type: " << data.type << ", state: START_NODE, ast: " << data.nodeType << std::endl;
      if (tree.setNodeType(g, (std::stringstream() << data.nodeType).str(), data.nodeId)) {
        horizontalOffset.startAtCurrent({}).wait(1).finish();//TODO TEMP remove me
        tree.updateNodes();
        if (shouldSetHideTokenCount >= 0) {
          tokenStream.pop(shouldSetHideTokenCount);
          shouldSetHideTokenCount = -1;
        }
      }
      break;
    case Data::END_NODE:
//      std::cout << "Parser - type: " << data.type << ", state: END_NODE, ast: " << data.nodeType << std::endl;
      tree.selectParentNode();
      horizontalOffset.startAtCurrent({}).wait(0.5).finish();//TODO TEMP remove me
      break;
    case Data::PARAM:
//      std::cout << "Parser - type: " << data.type << ", state: PARAM, param: {" << data.param.first << ", " << data.param.second << "}" << std::endl;
      tree.addTagToNode(g, data.param.first, data.param.second);
      tree.updateNodes();
//      horizontalOffset.startAtCurrent({}).wait(1).finish();//TODO TEMP remove me
      if (shouldSetHideTokenCount >= 0) {
        tokenStream.pop(shouldSetHideTokenCount);
        shouldSetHideTokenCount = -1;
      }
      break;
    case Data::ADD_CHILD:
//      std::cout << "Parser - type: " << data.type << ", state: ADD_CHILD, group: " << data.parserChildGroup << std::endl;
      tree.addNode(g, data.parserChildGroup);
      if (data.targetNodeType != ASTType::UNINITIALISED) {
        tree.setNodeType(g, (std::stringstream() << data.targetNodeType).str(), data.nodeId);
      } else {
        tree.setNodeType(g, "EXPRESSION", data.nodeId);
      }
      tree.updateNodes();
      if (shouldSetHideTokenCount >= 0) {
        tokenStream.pop(shouldSetHideTokenCount);
        shouldSetHideTokenCount = -1;
      }
//      horizontalOffset.startAtCurrent({}).wait(1).finish();//TODO TEMP remove me
      break;
    case Data::INSERT_NODE_BETWEEN_CHILD: {
//      std::cout << "Parser - type: " << data.type << ", state: INSERT_NODE_BETWEEN_CHILD, ast: " << data.nodeType << std::endl;
      tree.insertNodeBetweenChild(g, data.parserChildGroup, data.childNodeId);
      tree.setNodeType(g, (std::stringstream() << data.nodeType).str(), data.nodeId);
      tree.updateNodes();
//      horizontalOffset.startAtCurrent({}).wait(1).finish();//TODO TEMP remove me
      if (shouldSetHideTokenCount >= 0) {
        tokenStream.pop(shouldSetHideTokenCount);
        shouldSetHideTokenCount = -1;
      }
      break;
    }
    case Data::EXPECT:
//      std::cout << "Parser - type: " << data.type << ", state: EXPECT, token: " << data.targetToken << std::endl;
      break;
    case Data::EXPECT_PASS:
//      std::cout << "Parser - type: " << data.type << ", state: EXPECT_PASS, token: " << data.targetToken << std::endl;
//      tokenStream.pop();
//      shouldSetHideTokenCount = data.index;
      tokenStream.pop(data.index);
      break;
    case Data::EXPECT_FAIL:
//      std::cout << "Parser - type: " << data.type << ", state: EXPECT_FAIL, token: " << data.targetToken << std::endl;
      break;
    case Data::ACCEPT:
//      std::cout << "Parser - type: " << data.type << ", state: ACCEPT, token: " << data.targetToken << std::endl;
      break;
    case Data::ACCEPT_PASS:
//      std::cout << "Parser - type: " << data.type << ", state: ACCEPT_PASS, token: " << data.targetToken << std::endl;
//      tokenStream.pop();
//      shouldSetHideTokenCount = data.index;
      tokenStream.pop(data.index);
      break;
    case Data::ACCEPT_FAIL:
//      std::cout << "Parser - type: " << data.type << ", state: ACCEPT_FAIL, token: " << data.targetToken << std::endl;
      break;
  }
}

void VisualMain::handleCodeGenData(const Data& data) {
  switch (data.codeGenState) {
    case Data::CODE_GEN_UNINITIALISED:
//      std::cout << "Code Gen - type: " << data.type << ", state: CODE_GEN_UNINITIALISED" << std::endl;
      break;
    case Data::OUTPUT:
//      std::cout << "Code Gen - type: " << data.type << ", state: OUTPUT, string: `" << data.string << "`" << std::endl;
      instructionCode.add(data.string);
      horizontalOffset.startAtCurrent({}).wait(0.2).finish();//TODO TEMP remove me
      break;
    case Data::ENTER_NODE:
//      std::cout << "Code Gen - type: " << data.type << ", state: ENTER_NODE, ast: " << data.nodeType << std::endl;
      tree.selectNode(data.nodeId);
      break;
    case Data::EXIT_NODE:
//      std::cout << "Code Gen - type: " << data.type << ", state: EXIT_NODE, ast: " << data.nodeType << std::endl;
      tree.selectNode(data.nodeId);
      tree.selectParentNode();
      break;
    case Data::PUSH_CONSTANT:
      if (data.isNew) {
        tblConstants.add(g, data.id, data.string);
      }
      break;
    case Data::POP_CONSTANT:
      tblConstants.remove(data.id);
      break;
    case Data::SET_REG: {
      std::string reg = (std::stringstream() << data.reg).str();
      std::transform(reg.begin(), reg.end(), reg.begin(), ::toupper);
      if (data.loc) {
        std::string loc = (std::stringstream() << data.loc).str();
        tblRegisters.set(g, reg, loc);
      } else {
        tblRegisters.set(g, reg, "");
      }

      arrows.clear();

      horizontalOffset.startAtCurrent({}).wait(0.5).finish();//TODO TEMP remove me
      break;
    }
    case Data::SET_LOC: {
      std::string loc = (std::stringstream() << data.loc).str();
      if (data.reg != Register::NONE) {
        std::string reg = (std::stringstream() << data.reg).str();
        std::transform(reg.begin(), reg.end(), reg.begin(), ::toupper);
        if (tblLocations.has(loc)) {
          tblLocations.set(g, loc, reg);
        } else {
          tblLocations.add(g, loc, reg);
        }
      } else {
        tblLocations.remove(loc);
      }

      arrows.clear();

      horizontalOffset.startAtCurrent({}).wait(0.5).finish();//TODO TEMP remove me
      break;
    }
    case Data::SET_REG_AND_LOC: {
      std::string reg = (std::stringstream() << data.reg).str();
      std::transform(reg.begin(), reg.end(), reg.begin(), ::toupper);
      std::string loc = (std::stringstream() << data.loc).str();
      if (data.loc) {
        tblRegisters.set(g, reg, loc);
      } else {
        tblRegisters.set(g, reg, "");
      }
      if (data.reg != Register::NONE) {
        if (tblLocations.has(loc)) {
          tblLocations.set(g, loc, reg);
        } else {
          tblLocations.add(g, loc, reg);
        }
      } else {
        tblLocations.remove(loc);
      }

      arrows.clear();

      horizontalOffset.startAtCurrent({}).wait(0.5).finish();//TODO TEMP remove me
      break;
    }
    case Data::MOVE_REG: {
      std::string reg = (std::stringstream() << data.reg).str();
      std::transform(reg.begin(), reg.end(), reg.begin(), ::toupper);
      std::string reg2 = (std::stringstream() << data.reg2).str();
      std::transform(reg2.begin(), reg2.end(), reg2.begin(), ::toupper);

      love::Vector2 reg1Pos = tblRegisters.getPositionOf(reg);
      love::Vector2 reg2Pos = tblRegisters.getPositionOf(reg2);

      if (!data.keep) {
        arrows.clear();
      }
      arrows.emplace_back(reg1Pos, reg2Pos);

      horizontalOffset.startAtCurrent({}).wait(0.5).finish();//TODO TEMP remove me
      break;
    }
    case Data::MOVE_CONST: {
      std::string reg = (std::stringstream() << data.reg).str();
      std::transform(reg.begin(), reg.end(), reg.begin(), ::toupper);

      love::Vector2 nodePos = tree.getSelectedNodePosition();
      love::Vector2 regPos = tblRegisters.getPositionOf(reg);

      if (!data.keep) {
        arrows.clear();
      }
      arrows.emplace_back(nodePos, regPos);

      horizontalOffset.startAtCurrent({}).wait(0.5).finish();//TODO TEMP remove me
      break;
    }
  }
}

bool activeAnimations = false;

void VisualMain::update(double dt) {
  if (!activeAnimations) {
    requestNextData();
  }

  if (state == Data::Mode::LEXER) {
    if (!shouldShowAllChecklistHighlighting) {
      auto yPos = txtLexerCurrentPos.get().y;
      lexerChecklist.highlightUntilYPos = fmax(0, yPos);
    } else {
      lexerChecklist.highlightUntilYPos = -1;
    }

    sourceCode.update(dt);
    txtLexerCurrentPos.update(dt);
  }

  if (state == Data::Mode::LEXER || state == Data::Mode::PARSER) {
    tokenStream.update(dt);
  }

  if (state == Data::Mode::PARSER || state == Data::Mode::CODE_GEN || state == Data::Mode::FINISHED) {
    tree.update(dt);
  }

  if (hasParserSupport) {
    tblRegisters.position.x = tree.position.x + tree.frameSize.get().x;
    tblLocations.position.x = tree.position.x + tree.frameSize.get().x;
    tblConstants.position.x = tree.position.x + tree.frameSize.get().x;
  }

  if (state == Data::Mode::CODE_GEN || state == Data::Mode::FINISHED) {
    tblRegisters.update(dt);
    tblLocations.update(dt);
    tblConstants.update(dt);
    instructionCode.update(dt);
  }

  horizontalOffset.update(dt);

  activeAnimations = sourceCode.hasActiveAnimations()
      || tokenStream.hasActiveAnimations()
      || txtLexerCurrentPos.isActive()
      || horizontalOffset.isActive()
      || tblRegisters.hasActiveAnimations()
      || tblLocations.hasActiveAnimations()
      || tblConstants.hasActiveAnimations()
      || instructionCode.hasActiveAnimations();

  if (state == Data::Mode::CODE_GEN) {
    activeAnimations = activeAnimations
        || tree.hasActiveAnimations();
  }
}

void VisualMain::draw() {
  {
    Matrix4 mat = g->getTransform();
    mat.translate(10, 10);
    txtTitle->draw(g, mat);

    mat.translate((float) g->getWidth() - (float) txtTimeText->getWidth() - (10 * 2), 0);
    txtTimeText->draw(g, mat);
  }

  // Separator
  {
    std::vector<Vector2> lineVerts = {
        {0,                     titleHeight - (g->getLineWidth() / 2)},
        {(float) g->getWidth(), titleHeight - (g->getLineWidth() / 2)},
    };
    g->polyline(&lineVerts[0], lineVerts.size());
  }

  // Horizontal translation
  g->push(Graphics::StackType::STACK_TRANSFORM);
  g->translate(horizontalOffset.get(), 0);
  int xScissorOffset = horizontalOffset.get();

  if (state == Data::Mode::LEXER || hasLexerSupport) {
    // Left: source code
    sourceCode.draw(g, xScissorOffset);

    // Separator
    {
      std::vector<Vector2> lineVerts = {
          {lexerChecklist.position.x, titleHeight},
          {lexerChecklist.position.x, (float) g->getHeight()},
      };
      g->polyline(&lineVerts[0], lineVerts.size());
    }

    // Middle: checklist
    g->setScissor({
                      (int) lexerChecklist.position.x + xScissorOffset, (int) lexerChecklist.position.y,
                      (int) lexerChecklist.position.x, (int) ((float) g->getHeight() - lexerChecklist.position.y)
                  });
    lexerChecklist.draw(g);

    if (isLexerCurrentVisible) {
      Vector2 cursorPos = txtLexerCurrentPos.get();
      Vector4 highlightRect;
      if (lexerCurrentIsPeek) {
        highlightRect = sourceCode.getPeekHighlightRect();
      } else {
        highlightRect = sourceCode.getHighlightRect();
      }
      highlightRect.x = lexerChecklist.getCursorPosition().x;
      highlightRect.y = cursorPos.y - ((float) txtLexerCurrent->getHeight() / 2);
      Matrix4 cursorMat;
      cursorMat.translate(highlightRect.x, highlightRect.y);

      auto origCol = g->getColor();
      if (lexerCurrentIsPeek) {
        love::Colorf peekHighlightColour = love::Colorf(187 / 255.0f, 38 / 255.0f, 186 / 255.0f, 1);
        g->setColor(peekHighlightColour);
      } else {
        love::Colorf highlightColour = love::Colorf(187 / 255.0f, 186 / 255.0f, 38 / 255.0f, 1);
        g->setColor(highlightColour);
      }
      g->rectangle(Graphics::DrawMode::DRAW_FILL,
                   highlightRect.x, highlightRect.y,
                   highlightRect.z, highlightRect.w);
      g->setColor(origCol);

      if (shouldScissorLexerCurrent) {
        g->intersectScissor({
                                (int) highlightRect.x + xScissorOffset, (int) highlightRect.y,
                                (int) highlightRect.z, (int) highlightRect.w
                            });
      }
      txtLexerCurrent->draw(g, cursorMat);
    }
    g->setScissor();

    // Separator
    {
      std::vector<Vector2> lineVerts = {
          {tokenStream.position.x, titleHeight},
          {tokenStream.position.x, (float) g->getHeight()},
      };
      g->polyline(&lineVerts[0], lineVerts.size());
    }
  }

  if (state == Data::Mode::LEXER || state == Data::Mode::PARSER || hasParserSupport) {
    // Lex: Right: token stream
    // Parse: Left: token stream
    tokenStream.draw(g, xScissorOffset);
  }

  if (state == Data::Mode::PARSER || state == Data::Mode::CODE_GEN || state == Data::Mode::FINISHED) {
    // Separator
    {
      std::vector<Vector2> lineVerts = {
          {(float) g->getWidth(), titleHeight},
          {(float) g->getWidth(), (float) g->getHeight()},
      };
      g->polyline(&lineVerts[0], lineVerts.size());
    }

    // Parse: Right
    // Code Gen: Left
    tree.draw(g, xScissorOffset);

    // Separator
    {
      auto treeWidth = tree.frameSize.get().x;
      std::vector<Vector2> lineVerts = {
          {(float) g->getWidth() + treeWidth, titleHeight},
          {(float) g->getWidth() + treeWidth, (float) g->getHeight()},
      };
      g->polyline(&lineVerts[0], lineVerts.size());
    }
  }

  if (state == Data::Mode::CODE_GEN || state == Data::Mode::FINISHED) {
    auto treeWidth = tree.frameSize.get().x;
    auto tablesWidth = (float) g->getWidth() / 3;
    auto tablesHeight = 0;

    // Middle top: registers
    tblRegisters.draw(g, xScissorOffset);

    // Separator horizontal
    {
      tablesHeight += tblRegisters.frameSize.y;
      std::vector<Vector2> lineVerts = {
          {(float) g->getWidth() + treeWidth, titleHeight + tablesHeight},
          {(float) g->getWidth() + treeWidth + tablesWidth, titleHeight + tablesHeight},
      };
      g->polyline(&lineVerts[0], lineVerts.size());
    }

    // Middle middle: locations
    tblLocations.draw(g, xScissorOffset);

    // Separator horizontal
    {
      tablesHeight += tblLocations.frameSize.y;
      std::vector<Vector2> lineVerts = {
          {(float) g->getWidth() + treeWidth, titleHeight + tablesHeight},
          {(float) g->getWidth() + treeWidth + tablesWidth, titleHeight + tablesHeight},
      };
      g->polyline(&lineVerts[0], lineVerts.size());
    }

    // Middle bottom: constants
    tblConstants.draw(g, xScissorOffset);

    // Separator
    {
      std::vector<Vector2> lineVerts = {
          {(float) g->getWidth() + treeWidth + tablesWidth, titleHeight},
          {(float) g->getWidth() + treeWidth + tablesWidth, (float) g->getHeight()},
      };
      g->polyline(&lineVerts[0], lineVerts.size());
    }

    // Right: output
    instructionCode.draw(g, xScissorOffset);

    for (const auto& arrow : arrows) {
      auto vec = arrow.second - arrow.first;
      float angle = atan2(vec.y, vec.x) + (float) M_PI;

      static float len = 20;
      static float deg = 15;
      love::Vector2 p1 = {
          arrow.second.x - len * cos(angle - deg),
          arrow.second.y - len * sin(angle - deg)
      };
      love::Vector2 p2 = {
          arrow.second.x - len * cos(angle + deg),
          arrow.second.y - len * sin(angle + deg)
      };

      std::vector<Vector2> lineVerts = {
          arrow.first,
          arrow.second,
          p1,
          p2,
          arrow.second,
      };
      float origLineWidth = g->getLineWidth();
      g->setLineWidth(2);
      g->polyline(&lineVerts[0], lineVerts.size());
      g->setLineWidth(origLineWidth);
    }
  }

  g->pop();

  if (showError) {
    Matrix4 mat = g->getTransform();
    auto origCol = g->getColor();
    g->rectangle(Graphics::DrawMode::DRAW_LINE,
                 100, 100,
                 (float) txtError->getWidth() + 20, (float) txtError->getHeight() + 20);
    g->setColor(g->getBackgroundColor());
    g->rectangle(Graphics::DrawMode::DRAW_FILL,
        100, 100,
        (float) txtError->getWidth() + 20, (float) txtError->getHeight() + 20);
    g->setColor({1, 0, 0, 1});
    mat.translate(100 + 10, 100 + 10);
    txtError->draw(g, mat);
    g->setColor(origCol);
  }
}

void VisualMain::skipToNextSection() {
  // Disable all animations
  tweenNoAnimate = true;

  // Do one update cycle to force current animations to end.
  // HACK: temp set `activeAnimations` = true to ensure
  //     `requestNextData` doesn't get called before animations are flushed.
  bool activeAnimationsBak = activeAnimations;
  activeAnimations = true;
  update(0);
  activeAnimations = activeAnimationsBak;

  // Request next data (also re-enables animations) until next section is reached
  shouldSkipToNextSection = true;
  requestNextData();
}

void VisualMain::skipForwardOneStep() {
  // Disable all animations
  tweenNoAnimate = true;

  // Do one update cycle to force current animations to end.
  // HACK: temp set `activeAnimations` = true to ensure
  //     `requestNextData` doesn't get called before animations are flushed.
  bool activeAnimationsBak = activeAnimations;
  activeAnimations = true;
  update(0);
  activeAnimations = activeAnimationsBak;

  // Request next data (also re-enables animations)
  requestNextData();
}

void VisualMain::setTimeText(const std::string& timeStr) {
  txtTimeText->set(buildColoredString(timeStr, love::Colorf(1, 1, 1, 1)));
}
