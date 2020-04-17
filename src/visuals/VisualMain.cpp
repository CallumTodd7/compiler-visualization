//
// Created by Callum Todd on 2020/04/14.
//

#include <iostream>
#include "VisualMain.h"
#include "love2dShaders.h"
#include "love2dHelper.h"

VisualMain::VisualMain(ThreadSync* threadSync, Timer* timer)
    : threadSync(threadSync), timer(timer) {
  Graphics::defaultShaderCode[Shader::STANDARD_DEFAULT][Shader::LANGUAGE_GLSL3][0].source[ShaderStage::STAGE_VERTEX] = defaultVertexShaderCode;
  Graphics::defaultShaderCode[Shader::STANDARD_DEFAULT][Shader::LANGUAGE_GLSL3][0].source[ShaderStage::STAGE_PIXEL] = defaultPixelShaderCode;
  Graphics::defaultShaderCode[Shader::STANDARD_VIDEO][Shader::LANGUAGE_GLSL3][0].source[ShaderStage::STAGE_VERTEX] = defaultVertexShaderCode;
  Graphics::defaultShaderCode[Shader::STANDARD_VIDEO][Shader::LANGUAGE_GLSL3][0].source[ShaderStage::STAGE_PIXEL] = videoPixelShaderCode;
  Graphics::defaultShaderCode[Shader::STANDARD_ARRAY][Shader::LANGUAGE_GLSL3][0].source[ShaderStage::STAGE_VERTEX] = defaultVertexShaderCode;
  Graphics::defaultShaderCode[Shader::STANDARD_ARRAY][Shader::LANGUAGE_GLSL3][0].source[ShaderStage::STAGE_PIXEL] = arrayPixelShaderCode;

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
      case Data::Type::NOOP: break;
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

void VisualMain::init() {
  g->setActive(true);
  window->requestAttention(false);

  txtTitle = g->newText(g->getFont(), buildColoredString("Press [SPACE] to start", g));
  txtLexerCurrent = g->newText(g->getFont());
  txtLexerPeek = g->newText(g->getFont());
  txtLexerTextCursor = g->newText(g->getFont());

  sourceCode.init(g);
  sourceCode.position.x = 10;
  sourceCode.position.y = 40;

  lexerChecklist.alignment = Alignment::RIGHT;
  lexerChecklist.enableCursor = false;

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
    sourceCode.highlightPeek(data.lexerContextStart.lineNumber, data.lexerContextStart.characterPos,
                             data.lexerContextEnd.lineNumber, data.lexerContextEnd.characterPos);
  } else {
    sourceCode.highlight(data.lexerContextStart.lineNumber, data.lexerContextStart.characterPos,
                         data.lexerContextEnd.lineNumber, data.lexerContextEnd.characterPos);
  }
  //if (data.lexerState == Data::LexerState::END_OF_FILE)
  //if (data.lexerState == Data::LexerState::UNKNOWN)

  if (data.peekedChar > 0) {
    txtLexerPeek->set(buildColoredString(std::string(1, data.peekedChar), g));
  }

  if (data.lexerState == Data::LexerState::WORD_UPDATE
  // TODO The below END_* should be removed; this will make way for the floating token
      || data.lexerState == Data::LexerState::END_NUMBER
      || data.lexerState == Data::LexerState::END_STRING
      || data.lexerState == Data::LexerState::END_ALPHA_KEYWORD
      || data.lexerState == Data::LexerState::END_ALPHA_IDENT
      || data.lexerState == Data::LexerState::END_OP
      ) {
    txtLexerCurrent->set(buildColoredString(data.string, g));
  } else {
    txtLexerCurrent->clear();
  }

  if (data.lexerState == Data::LexerState::NEW_TOKEN) {
    lexerChecklist.reset();
    txtLexerCurrent->clear();
    return;
  }

  if (data.lexerState == Data::LexerState::END_OF_FILE) {
    lexerChecklist.accept(0, true);
  } else if (data.lexerState == Data::LexerState::START_NUMBER) {
    lexerChecklist.accept(1, false);
  } else if (data.lexerState == Data::LexerState::END_NUMBER) {
    lexerChecklist.accept(1, true);
  } else if (data.lexerState == Data::LexerState::START_STRING) {
    lexerChecklist.accept(2, false);
  } else if (data.lexerState == Data::LexerState::END_STRING) {
    lexerChecklist.accept(2, true);
  } else if (data.lexerState == Data::LexerState::START_ALPHA) {
    lexerChecklist.accept(3, false);
  } else if (data.lexerState == Data::LexerState::END_ALPHA_KEYWORD) {
    lexerChecklist.accept(4, true);
  } else if (data.lexerState == Data::LexerState::END_ALPHA_IDENT) {
    lexerChecklist.accept(5, true);
  } else if (data.lexerState == Data::LexerState::START_OP) {
    lexerChecklist.accept(6, false);
  } else if (data.lexerState == Data::LexerState::END_OP) {
    lexerChecklist.accept(6, true);
  } else if (data.lexerState == Data::LexerState::UNKNOWN) {
    lexerChecklist.accept(7, true);
  }

//  if (data.lexerState == Data::LexerState::END_OF_FILE
//      || data.lexerState == Data::LexerState::START_NUMBER
//      || data.lexerState == Data::LexerState::START_STRING
//      || data.lexerState == Data::LexerState::START_ALPHA
//      || data.lexerState == Data::LexerState::START_OP) {
//    if (data.lexerState == Data::LexerState::END_OF_FILE) {
//      lexerChecklist.accept();
//    } else {
//      lexerChecklist.next();
//    }
//
//    if (data.lexerState == Data::LexerState::START_NUMBER) {
//      lexerChecklist.accept();
//    } else {
//      lexerChecklist.next();
//    }
//
//    if (data.lexerState == Data::LexerState::START_STRING) {
//      lexerChecklist.accept();
//    } else {
//      lexerChecklist.next();
//    }
//
//    if (data.lexerState == Data::LexerState::START_ALPHA) {
//      lexerChecklist.accept();
//    } else {
//      lexerChecklist.next();
//      lexerChecklist.next();
//      lexerChecklist.next();
//    }
//
//    if (data.lexerState == Data::LexerState::START_OP) {
//      lexerChecklist.accept();
//    } else {
//      lexerChecklist.next();
//      lexerChecklist.accept();
//    }
//  }
}

void VisualMain::handleParserData(const Data& data) {

}

void VisualMain::handleCodeGenData(const Data& data) {

}

int pastTime = -1;
bool activeTweens = false;
void VisualMain::update(double dt) {
//  if ((int) Timer::getTime() != pastTime) {
//    requestNextData();
//    pastTime = (int) Timer::getTime();
//    return;
//  }

  if (pastTime > 300 && !activeTweens) {
//    std::cout << "Req data" << std::endl;
    requestNextData();
    pastTime = -1;
  }
  pastTime++;

  sourceCode.update(dt, 0);

  activeTweens = sourceCode.hasActiveTweens();
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
    float padding = 5;
    mat = g->getTransform();
    float y = 10;
    mat.translate((float) g->getWidth() / 2.0f + padding, y + padding);
    txtLexerPeek->draw(g, mat);

    mat.translate(50, 0);
    txtLexerCurrent->draw(g, mat);
    Vector2 rectSize = {300, g->getFont()->getHeight() + (padding * 2.0f)};
    g->push(Graphics::StackType::STACK_ALL);
    g->setColor(Colorf(1, 1, 0, 1));
    g->rectangle(Graphics::DrawMode::DRAW_LINE, (float) g->getWidth() / 2.0f, y, rectSize.x, rectSize.y);
    g->pop();
    y += rectSize.y;
    y += padding * 2.0f;

    lexerChecklist.draw(g, {(float) g->getWidth() / 2.0f, y});
    Vector2 cursorPos = lexerChecklist.getDrawnCursorPosition();

    Matrix4 cursorMat = g->getTransform();
    cursorMat.translate(cursorPos.x, cursorPos.y - ((float)g->getFont()->getHeight() / 2));
    txtLexerTextCursor->draw(g, cursorMat);
//    g->rectangle(Graphics::DrawMode::DRAW_FILL, cursorPos.x, cursorPos.y - 5, 10, 10);

    // Right: token stream
  }
}
