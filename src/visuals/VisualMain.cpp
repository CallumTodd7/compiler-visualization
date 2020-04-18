//
// Created by Callum Todd on 2020/04/14.
//

#include <iostream>
#include "VisualMain.h"
#include "love2dShaders.h"
#include "love2dHelper.h"

// Font
#include "SourceCodePro-regular.ttf.h"

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

void VisualMain::requestNextData() {
  threadSync->getData([&](const Data& data) {
//    std::cout << "Data - type: " << data.type << ", mode: " << data.mode << std::endl;
    switch (data.type) {
      case Data::Type::NOOP:
        break;
      case Data::Type::MODE_CHANGE:
        setupNewMode(data);
        break;
      case Data::Type::SPECIFIC:
        switch (data.mode) {
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
        }
        break;
    }
  });
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

  fontSourceCodeProRegular20 = genFont<SourceCodeProFontData>(20, TrueTypeRasterizer::Hinting::HINTING_NORMAL);
  g->setFont(fontSourceCodeProRegular20);

  txtTitle = g->newText(g->getFont(), buildColoredString("Press [SPACE] to start", g));
  txtLexerCurrent = g->newText(g->getFont());

  sourceCode.init(g);
  sourceCode.position.x = 10;
  sourceCode.position.y = 60;

  lexerChecklist.alignment = Alignment::RIGHT;
  lexerChecklist.enableCursor = false;
  lexerChecklist.cursorPadding = 20;
  auto charWidth = (float) g->getFont()->getWidth('0');
  lexerChecklist.cursorWidth = charWidth;
  lexerChecklist.indentSize = charWidth * 2;

  lexerChecklist.add(g, "End of file");
  lexerChecklist.add(g, "Digit");
  lexerChecklist.add(g, "String");
  auto alphaId = lexerChecklist.add(g, "Alpha");
  lexerChecklist.add(g, "Keyword", alphaId);
  lexerChecklist.add(g, "Identifier", alphaId);
  lexerChecklist.add(g, "Operator");
  lexerChecklist.add(g, "Error: Unknown token");
}

void VisualMain::setupNewMode(const Data& data) {
  state = data.mode;
  switch (state) {
    case Data::LEXER:
      txtTitle->set(buildColoredString("Lexing", g));
      sourceCode.load(data.filepath);
      break;
    case Data::PARSER:
      txtTitle->set(buildColoredString("Parsing", g));
      sourceCode.position.x += (float) g->getWidth() / 2.0f;
      break;
    case Data::CODE_GEN:
      txtTitle->set(buildColoredString("Code Generation", g));
      break;
    case Data::FINISHED:
      txtTitle->set(buildColoredString("Done!", g));
      break;
  }
}

void VisualMain::handleLexerData(const Data& data) {
  if (data.lexerState == Data::LexerState::NEW_TOKEN) {
    txtLexerCurrentPos.set({
                               lexerChecklist.getCursorPosition().x,
                               -(g->getFont()->getHeight() + lexerChecklist.padding)
                           });
    sourceCode.highlightPeek(data.lexerContextStart.lineNumber, data.lexerContextStart.characterPos,
                             data.lexerContextEnd.lineNumber, data.lexerContextEnd.characterPos);
    lexerCurrentIsPeek = true;
  } else if (data.lexerState == Data::LexerState::WORD_UPDATE) {
    sourceCode.highlight(data.lexerContextStart.lineNumber, data.lexerContextStart.characterPos,
                         data.lexerContextEnd.lineNumber, data.lexerContextEnd.characterPos);
    lexerCurrentIsPeek = false;
  }

  if (data.peekedChar > 0) {
    txtLexerCurrent->set(buildColoredString(std::string(1, data.peekedChar), g));
  }

  if (data.lexerState == Data::LexerState::WORD_UPDATE
      ) {
    txtLexerCurrent->set(buildColoredString(data.string, g));
    if (data.string.size() > 1) {
      shouldScissorLexerCurrent = true;
    }
  }

  if (data.lexerState == Data::LexerState::NEW_TOKEN) {
    lexerChecklist.reset();
    return;
  }

  auto moveToPos = [&](int travelIndexDistance, double wait = 0.0) {
    shouldShowAllChecklistHighlighting = false;
    txtLexerCurrentPos
        .startAtCurrent({})
        .goTo(lexerChecklist.getCursorPosition(), 0.5 * travelIndexDistance)
        .callback([&] {
          shouldScissorLexerCurrent = false;
          sourceCode.unhighlightPeek();
          shouldShowAllChecklistHighlighting = true;
        })
        .wait(wait)
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
  } else if (data.lexerState == Data::LexerState::START_STRING) {
    lexerChecklist.accept(2, false);
    moveToPos(2);
  } else if (data.lexerState == Data::LexerState::END_STRING) {
    lexerChecklist.accept(2, true);
  } else if (data.lexerState == Data::LexerState::START_ALPHA) {
    lexerChecklist.accept(3, false);
    moveToPos(3);
  } else if (data.lexerState == Data::LexerState::END_ALPHA_KEYWORD) {
    lexerChecklist.accept(4, true);
    moveToPos(1, 1.0);
  } else if (data.lexerState == Data::LexerState::END_ALPHA_IDENT) {
    lexerChecklist.accept(5, true);
    moveToPos(2, 1.0);
  } else if (data.lexerState == Data::LexerState::START_OP) {
    lexerChecklist.accept(6, false);
    moveToPos(6);
  } else if (data.lexerState == Data::LexerState::END_OP) {
    lexerChecklist.accept(6, true);
  } else if (data.lexerState == Data::LexerState::UNKNOWN) {
    lexerChecklist.accept(7, true);
    moveToPos(7);
  }
}

void VisualMain::handleParserData(const Data& data) {

}

void VisualMain::handleCodeGenData(const Data& data) {

}

bool activeAnimations = false;

void VisualMain::update(double dt) {
  if (!activeAnimations) {
    requestNextData();
  }

  sourceCode.update(dt, 0);
  txtLexerCurrentPos.update(dt);

  if (!shouldShowAllChecklistHighlighting) {
    auto yPos = txtLexerCurrentPos.get().y;
    lexerChecklist.highlightUntilYPos = fmax(0, yPos);
  } else {
    lexerChecklist.highlightUntilYPos = -1;
  }

  activeAnimations = sourceCode.hasActiveAnimations()
      || txtLexerCurrentPos.isActive();
}

void VisualMain::draw() {
//  g->rectangle(Graphics::DrawMode::DRAW_LINE, 10, 10, 100, 100);

  Matrix4 mat = g->getTransform();
  mat.translate(10, 10);
  txtTitle->draw(g, mat);

  if (state == Data::Mode::LEXER || state == Data::Mode::PARSER) {
    // Left: source code
    sourceCode.draw(g);

    // Middle: checklist
    float y = 40 + (g->getFont()->getHeight() + lexerChecklist.padding);

    lexerChecklist.draw(g, {(float) g->getWidth() / 2.0f, y});

    Vector2 cursorPos = txtLexerCurrentPos.get();
    Vector4 highlightRect;
    if (lexerCurrentIsPeek) {
      highlightRect = sourceCode.getPeekHighlightRect();
    } else {
      highlightRect = sourceCode.getHighlightRect();
    }
    highlightRect.x = ((float) g->getWidth() / 2.0f) + cursorPos.x;
    highlightRect.y = y + cursorPos.y - ((float) txtLexerCurrent->getHeight() / 2);
    Matrix4 cursorMat = g->getTransform();
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
      g->setScissor({
                        (int) highlightRect.x, (int) highlightRect.y,
                        (int) highlightRect.z, (int) highlightRect.w
                    });
    }
    txtLexerCurrent->draw(g, cursorMat);
    g->setScissor();

    // Right: token stream
  }
}
