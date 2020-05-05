//
// Created by Callum Todd on 2020/02/21.
//

#include <iostream>
#include <sstream>
#include "Parser.h"
#include "AST.h"
#include "Lexer.h"
#include "../Data.h"


Parser::Parser(const std::function<void(const Data&)>& ready, std::vector<Token> tokenStream)
    : ready(ready), tokenStream(std::move(tokenStream)) {
}

Token* Parser::currentToken() {
  if (tokenStreamIndex >= tokenStream.size()) return nullptr;
  return &tokenStream[tokenStreamIndex];
}

Token* Parser::nextToken() {
  if (tokenStreamIndex >= tokenStream.size()) return nullptr;
  return &tokenStream[tokenStreamIndex++];
}

Token* Parser::peekToken() {
  if (tokenStreamIndex + 1 >= tokenStream.size()) return nullptr;
  return &tokenStream[tokenStreamIndex + 1];
}

const Token* Parser::expect(Token::Type type) {
  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .parserState = Data::ParserState::EXPECT,
            .index = tokenStreamIndex,
            .targetToken = type,
        });

  const Token* token = currentToken();
  if (token && token->type == type) {
    ready({
              .mode = Data::Mode::PARSER,
              .type = Data::Type::SPECIFIC,
              .parserState = Data::ParserState::EXPECT_PASS,
              .index = tokenStreamIndex,
              .targetToken = type,
          });

    return nextToken();
  }

  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .parserState = Data::ParserState::EXPECT_FAIL,
            .index = tokenStreamIndex,
            .targetToken = type,
        });

  std::cout << token->lexerContext
            << " Parser: Unexpected token. Got " << token->type << ", expected " << type << "." << std::endl;
  throw std::exception();
}

const Token* Parser::accept(Token::Type type) {
  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .parserState = Data::ParserState::ACCEPT,
            .index = tokenStreamIndex,
            .targetToken = type,
        });

  const Token* token = currentToken();
  if (token && token->type == type) {
    ready({
              .mode = Data::Mode::PARSER,
              .type = Data::Type::SPECIFIC,
              .parserState = Data::ParserState::ACCEPT_PASS,
              .index = tokenStreamIndex,
              .targetToken = type,
          });

    return nextToken();
  }

  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .parserState = Data::ParserState::ACCEPT_FAIL,
            .index = tokenStreamIndex,
            .targetToken = type,
        });

  return nullptr;
}

const Token* Parser::accept(const std::vector<Token::Type>& types) {
  const Token* token = currentToken();
  if (token) {
    for (auto& type : types) {
      ready({
                .mode = Data::Mode::PARSER,
                .type = Data::Type::SPECIFIC,
                .parserState = Data::ParserState::ACCEPT,
                .index = tokenStreamIndex,
                .targetToken = type,
            });

      if (token->type == type) {
        ready({
                  .mode = Data::Mode::PARSER,
                  .type = Data::Type::SPECIFIC,
                  .parserState = Data::ParserState::ACCEPT_PASS,
                  .index = tokenStreamIndex,
                  .targetToken = type,
              });
        return nextToken();
      }

      ready({
                .mode = Data::Mode::PARSER,
                .type = Data::Type::SPECIFIC,
                .parserState = Data::ParserState::ACCEPT_FAIL,
                .index = tokenStreamIndex,
                .targetToken = type,
            });
    }
  }

  return nullptr;
}

ASTBlock* Parser::parse() {
  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .parserState = Data::ParserState::ADD_CHILD,
            .targetNodeType = ASTType::BLOCK,
        });

  auto node = new ASTBlock();
  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::START_NODE,
  });

  node->parent = nullptr;

  blockScopeStack.push(node);

  while (tokenStreamIndex < tokenStream.size()) {
    bool isExternal = currentToken()->type == Token::TOKEN_KEYWORD_EXTERN;

    ready({
              .mode = Data::Mode::PARSER,
              .type = Data::Type::SPECIFIC,
              .nodeType = node->type,
              .parserState = Data::ParserState::ADD_CHILD,
              .targetNodeType = ASTType::PROC_DECL,
              .parserChildGroup = "statements",
          });
    node->statements.push_back(parseProcedureDeclaration(isExternal));
  }

  blockScopeStack.pop();

  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::END_NODE,
        });
  return node;
}

ASTBlock* Parser::parseBlock() {
  auto node = new ASTBlock();
  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::START_NODE,
        });

  node->parent = blockScopeStack.top();

  expect(Token::Type::TOKEN_BRACE_OPEN);
  blockScopeStack.push(node);

  while (!accept(Token::Type::TOKEN_BRACE_CLOSE)) {
    ready({
              .mode = Data::Mode::PARSER,
              .type = Data::Type::SPECIFIC,
              .nodeType = node->type,
              .parserState = Data::ParserState::ADD_CHILD,
              .parserChildGroup = "statements",
          });
    node->statements.push_back(parseStatement());
  }

  blockScopeStack.pop();

  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::END_NODE,
        });
  return node;
}

ASTProcedure* Parser::parseProcedureDeclaration(bool isExternal) {
  auto node = new ASTProcedure();
  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::START_NODE,
        });

  node->isExternal = isExternal;
  if (isExternal) {
    expect(Token::Type::TOKEN_KEYWORD_EXTERN);
  }
  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::PARAM,
            .param = {"isExternal", node->isExternal ? "true" : "false"},
        });

  node->returnType = parseTypeSpecifier();
  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::PARAM,
            .param = {"returnType", (std::stringstream() << node->returnType).str()},
        });

  //TODO replace with proc call
  const Token* procIdentToken = expect(Token::Type::TOKEN_IDENTIFIER);
  node->ident = std::string(procIdentToken->value.stringValue.data,
                            procIdentToken->value.stringValue.count);//TODO use atom
  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::PARAM,
            .param = {"identifier", node->ident},
        });

  expect(Token::Type::TOKEN_PARENTHESIS_OPEN);

  while (!accept(Token::Type::TOKEN_PARENTHESIS_CLOSE)) {
    ready({
              .mode = Data::Mode::PARSER,
              .type = Data::Type::SPECIFIC,
              .nodeType = node->type,
              .parserState = Data::ParserState::ADD_CHILD,
              .targetNodeType = ASTType::VARIABLE_DECL,
              .parserChildGroup = "parameters",
          });
    node->parameters.push_back(parseVariableDeclaration(true));
    if (currentToken()->type != Token::Type::TOKEN_PARENTHESIS_CLOSE) {
      expect(Token::Type::TOKEN_COMMA);
    }
  }

  if (!isExternal) {
    ready({
              .mode = Data::Mode::PARSER,
              .type = Data::Type::SPECIFIC,
              .nodeType = node->type,
              .parserState = Data::ParserState::ADD_CHILD,
              .targetNodeType = ASTType::BLOCK,
              .parserChildGroup = "body",
          });
    node->block = parseBlock();
  }

  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::END_NODE,
        });
  return node;
}

ASTVariableDeclaration* Parser::parseVariableDeclaration(bool declaratorOnly) {
  auto node = new ASTVariableDeclaration();
  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::START_NODE,
        });

  node->dataType = parseTypeSpecifier();
  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::PARAM,
            .param = {"dataType", (std::stringstream() << node->dataType).str()},
        });

  //TODO replace with proc call
  const Token* procIdentToken = expect(Token::Type::TOKEN_IDENTIFIER);
  node->ident = std::string(procIdentToken->value.stringValue.data,
                            procIdentToken->value.stringValue.count);//TODO use atom
  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::PARAM,
            .param = {"identifier", node->ident},
        });

  if (!declaratorOnly) {
    if (accept(Token::Type::TOKEN_ASSIGN)) {
      ready({
                .mode = Data::Mode::PARSER,
                .type = Data::Type::SPECIFIC,
                .nodeType = node->type,
                .parserState = Data::ParserState::ADD_CHILD,
                .parserChildGroup = "initialValueExpression",
            });
      node->initialValueExpression = parseExpression();
    }

    expect(Token::Type::TOKEN_SEMICOLON);
  }

  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::END_NODE,
        });
  return node;
}

ASTVariableAssignment* Parser::parseVariableAssignment() {
  auto node = new ASTVariableAssignment();
  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::START_NODE,
        });

  //TODO replace with proc call
  const Token* procIdentToken = expect(Token::Type::TOKEN_IDENTIFIER);
  node->ident = std::string(procIdentToken->value.stringValue.data,
                            procIdentToken->value.stringValue.count);//TODO use atom
  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::PARAM,
            .param = {"identifier", node->ident},
        });

  expect(Token::Type::TOKEN_ASSIGN);

  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::ADD_CHILD,
            .parserChildGroup = "newValueExpression",
        });
  node->newValueExpression = parseExpression();

  expect(Token::Type::TOKEN_SEMICOLON);

  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::END_NODE,
        });
  return node;
}

ASTProcedureCall* Parser::parseProcedureCall() {
  auto node = new ASTProcedureCall();
  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::START_NODE,
        });

  //TODO replace with proc call
  const Token* procIdentToken = expect(Token::Type::TOKEN_IDENTIFIER);
  node->ident = std::string(procIdentToken->value.stringValue.data,
                            procIdentToken->value.stringValue.count);//TODO use atom
  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::PARAM,
            .param = {"identifier", node->ident},
        });

  expect(Token::Type::TOKEN_PARENTHESIS_OPEN);

  while (!accept(Token::Type::TOKEN_PARENTHESIS_CLOSE)) {
    ready({
              .mode = Data::Mode::PARSER,
              .type = Data::Type::SPECIFIC,
              .nodeType = node->type,
              .parserState = Data::ParserState::ADD_CHILD,
              .parserChildGroup = "parameter",
          });
    node->parameters.push_back(parseExpression());
    if (currentToken()->type != Token::Type::TOKEN_PARENTHESIS_CLOSE) {
      expect(Token::Type::TOKEN_COMMA);
    }
  }

  expect(Token::Type::TOKEN_SEMICOLON);

  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::END_NODE,
        });
  return node;
}

ASTStatement* Parser::parseStatement() {
  const Token* token = currentToken();

  switch (token->type) {
    case Token::Type::TOKEN_BRACE_OPEN:
      return parseBlock();
    case Token::Type::TOKEN_KEYWORD_WHILE:
      return parseWhile();
    case Token::Type::TOKEN_KEYWORD_IF:
      return parseIf();
    case Token::Type::TOKEN_KEYWORD_CONTINUE:
      return parseContinue();
    case Token::Type::TOKEN_KEYWORD_BREAK:
      return parseBreak();
    case Token::Type::TOKEN_KEYWORD_RETURN:
      return parseReturn();
    case Token::Type::TOKEN_KEYWORD_EXTERN:
      return parseProcedureDeclaration(true);
    case Token::Type::TOKEN_KEYWORD_VOID:
    case Token::Type::TOKEN_KEYWORD_U8:
    case Token::Type::TOKEN_KEYWORD_S8:
      return parseVariableDeclaration();
    case Token::Type::TOKEN_IDENTIFIER:
      if (peekToken()->type == Token::Type::TOKEN_PARENTHESIS_OPEN) {
        return parseProcedureCall();
      } else {
        return parseVariableAssignment();
      }
    default:
      std::cout << token->lexerContext << " Parser: Expected statement, got " << *token << "." << std::endl;
      throw std::exception();
  }
}

ASTWhile* Parser::parseWhile() {
  auto node = new ASTWhile();
  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::START_NODE,
        });

  expect(Token::Type::TOKEN_KEYWORD_WHILE);

  expect(Token::Type::TOKEN_PARENTHESIS_OPEN);
  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::ADD_CHILD,
            .parserChildGroup = "conditional",
        });
  node->conditional = parseExpression();
  expect(Token::Type::TOKEN_PARENTHESIS_CLOSE);

  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::ADD_CHILD,
            .targetNodeType = ASTType::BLOCK,
            .parserChildGroup = "body",
        });
  node->body = parseBlock();

  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::END_NODE,
        });
  return node;
}

ASTIf* Parser::parseIf() {
  auto node = new ASTIf();
  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::START_NODE,
        });

  expect(Token::Type::TOKEN_KEYWORD_IF);

  expect(Token::Type::TOKEN_PARENTHESIS_OPEN);
  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::ADD_CHILD,
            .parserChildGroup = "conditional",
        });
  node->conditional = parseExpression();
  expect(Token::Type::TOKEN_PARENTHESIS_CLOSE);

  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::ADD_CHILD,
            .targetNodeType = ASTType::BLOCK,
            .parserChildGroup = "'true' body",
        });
  node->trueStatement = parseBlock();

  if (accept(Token::Type::TOKEN_KEYWORD_ELSE)) {
    if (currentToken()->type == Token::Type::TOKEN_BRACE_OPEN) {
      ready({
                .mode = Data::Mode::PARSER,
                .type = Data::Type::SPECIFIC,
                .nodeType = node->type,
                .parserState = Data::ParserState::ADD_CHILD,
                .targetNodeType = ASTType::BLOCK,
                .parserChildGroup = "'false' body",
            });
      node->falseStatement = parseBlock();
    } else {
      ready({
                .mode = Data::Mode::PARSER,
                .type = Data::Type::SPECIFIC,
                .nodeType = node->type,
                .parserState = Data::ParserState::ADD_CHILD,
                .targetNodeType = ASTType::IF,
                .parserChildGroup = "'false' body",
            });
      node->falseStatement = parseIf();
    }
  } else {
    ready({
              .mode = Data::Mode::PARSER,
              .type = Data::Type::SPECIFIC,
              .nodeType = node->type,
              .parserState = Data::ParserState::PARAM,
              .param = {"'false' body", "null"},
          });
    node->falseStatement = nullptr;
  }

  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::END_NODE,
        });
  return node;
}

ASTContinue* Parser::parseContinue() {
  auto node = new ASTContinue();
  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::START_NODE,
        });

  expect(Token::Type::TOKEN_KEYWORD_CONTINUE);
  expect(Token::Type::TOKEN_SEMICOLON);

  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::END_NODE,
        });
  return node;
}

ASTBreak* Parser::parseBreak() {
  auto node = new ASTBreak();
  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::START_NODE,
        });

  expect(Token::Type::TOKEN_KEYWORD_BREAK);
  expect(Token::Type::TOKEN_SEMICOLON);

  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::END_NODE,
        });
  return node;
}

ASTReturn* Parser::parseReturn() {
  auto node = new ASTReturn();
  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::START_NODE,
        });

  expect(Token::Type::TOKEN_KEYWORD_RETURN);
  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::ADD_CHILD,
            .parserChildGroup = "return value",
        });
  node->expression = parseExpression();
  expect(Token::Type::TOKEN_SEMICOLON);

  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .parserState = Data::ParserState::END_NODE,
        });
  return node;
}

ASTExpression* Parser::parseExpression() {
  ready({
            .mode = Data::Mode::PARSER,
            .type = Data::Type::SPECIFIC,
            .parserState = Data::ParserState::END_NODE,
        });
  return parseLogicalOrExp();
}

ASTExpression* Parser::parseLogicalOrExp() {
  auto node = parseLogicalAndExp();

  const Token* opToken = nullptr;
  while ((opToken = accept(Token::Type::TOKEN_LOGICAL_OR))) {
    auto rightNode = parseLogicalAndExp();

    auto newNode = new ASTBinOp();
    newNode->left = node;
    newNode->op = getOpFromToken(opToken);
    newNode->right = rightNode;

    node = newNode;
  }

  return node;
}

ASTExpression* Parser::parseLogicalAndExp() {
  auto node = parseEqualityExp();

  const Token* opToken = nullptr;
  while ((opToken = accept(Token::Type::TOKEN_LOGICAL_AND))) {
    auto rightNode = parseEqualityExp();

    auto newNode = new ASTBinOp();
    newNode->left = node;
    newNode->op = getOpFromToken(opToken);
    newNode->right = rightNode;

    node = newNode;
  }

  return node;
}

ASTExpression* Parser::parseEqualityExp() {
  auto node = parseRelationalExp();

  const Token* opToken = nullptr;
  while ((opToken = accept({
                               Token::Type::TOKEN_EQUALS,
                               Token::Type::TOKEN_NOT_EQUALS
                           }))) {
    auto rightNode = parseRelationalExp();

    auto newNode = new ASTBinOp();
    newNode->left = node;
    newNode->op = getOpFromToken(opToken);
    newNode->right = rightNode;

    node = newNode;
  }

  return node;
}

ASTExpression* Parser::parseRelationalExp() {
  auto node = parseAdditiveExp();

  const Token* opToken = nullptr;
  while ((opToken = accept({
                               Token::Type::TOKEN_LESS_THAN,
                               Token::Type::TOKEN_LESS_THAN_OR_EQUAL,
                               Token::Type::TOKEN_GREATER_THAN,
                               Token::Type::TOKEN_GREATER_THAN_OR_EQUAL,
                           }))) {
    auto rightNode = parseAdditiveExp();

    auto newNode = new ASTBinOp();
    newNode->left = node;
    newNode->op = getOpFromToken(opToken);
    newNode->right = rightNode;

    node = newNode;
  }

  return node;
}

ASTExpression* Parser::parseAdditiveExp() {
  auto node = parseTerm();

  const Token* opToken = nullptr;
  while ((opToken = accept({
                               Token::Type::TOKEN_ADD,
                               Token::Type::TOKEN_MINUS
                           }))) {
    auto rightNode = parseTerm();

    auto newNode = new ASTBinOp();
    newNode->left = node;
    newNode->op = getOpFromToken(opToken);
    newNode->right = rightNode;

    node = newNode;
  }

  return node;
}

ASTExpression* Parser::parseTerm() {
  auto node = parseUnaryFactor();

  const Token* opToken = nullptr;
  while ((opToken = accept({
                               Token::Type::TOKEN_MULTIPLY,
                               Token::Type::TOKEN_DIVIDE
                           }))) {
    auto rightNode = parseUnaryFactor();

    auto newNode = new ASTBinOp();
    newNode->left = node;
    newNode->op = getOpFromToken(opToken);
    newNode->right = rightNode;

    node = newNode;
  }

  return node;
}

ASTExpression* Parser::parseUnaryFactor() {
  const Token* opToken = accept({
                                    Token::Type::TOKEN_ADD,
                                    Token::Type::TOKEN_MINUS,
                                    Token::Type::TOKEN_LOGICAL_NOT
                                });
  if (opToken) {
    auto node = new ASTUnaryOp();
    node->child = parseFactor();
    node->op = getOpFromToken(opToken);

    return node;
  }

  return parseFactor();
}

ASTExpression* Parser::parseFactor() {
  const Token* peekedToken = currentToken();

  switch (peekedToken->type) {
    case Token::Type::TOKEN_PARENTHESIS_OPEN: {
      expect(Token::Type::TOKEN_PARENTHESIS_OPEN);
      auto expression = parseExpression();
      expect(Token::Type::TOKEN_PARENTHESIS_CLOSE);

      return expression;
    }
    case Token::Type::TOKEN_IDENTIFIER: {
      if (peekToken()->type == Token::Type::TOKEN_PARENTHESIS_OPEN) {
        // Proc call

        std::cout << peekedToken->lexerContext
                  << " Parser: procedure calls are not implemented yet" << std::endl;
        throw std::exception();
      } else {
        // Ident

        auto token = expect(Token::Type::TOKEN_IDENTIFIER);
        if (token->valueType != Token::ValueType::STRING) {
          std::cout << token->lexerContext
                    << " Parser: variable identifier cannot be an integer" << std::endl;
          throw std::exception();
        }

        auto node = new ASTVariableIdent();
        node->ident = std::string(token->value.stringValue.data,
                                  token->value.stringValue.count);//TODO use atom

        return node;
      }
    }
    case Token::Type::TOKEN_INTEGER: {
      auto token = expect(Token::Type::TOKEN_INTEGER);

      auto node = new ASTLiteral();
      node->valueType = ASTLiteral::ValueType::INTEGER;
      node->value.integerData = token->value.integerValue;
      return node;
    }
    case Token::Type::TOKEN_STRING: {
      auto token = expect(Token::Type::TOKEN_STRING);

      auto node = new ASTLiteral();
      node->valueType = ASTLiteral::ValueType::STRING;
      // TODO cleanup
      node->value.stringData.count = token->value.stringValue.count;
      node->value.stringData.data = token->value.stringValue.data;
      return node;
    }
    default: {
      std::cout << *peekedToken << " Parser: Is not valid factor" << std::endl;
      throw std::exception();
    }
  }
}

DataType Parser::parseTypeSpecifier() {
  if (accept(Token::Type::TOKEN_KEYWORD_VOID)) {
    return DataType::VOID;
  } else if (accept(Token::Type::TOKEN_KEYWORD_U8)) {
    return DataType::U8;
  } else if (accept(Token::Type::TOKEN_KEYWORD_S8)) {
    return DataType::S8;
  }

  auto token = currentToken();
  std::cout << token->lexerContext
            << " Parser: Unexpected token. Got " << token->type << ", expected a type specifier." << std::endl;
  throw std::exception();
}

ExpressionOperatorType Parser::getOpFromToken(const Token* token) {
  switch (token->type) {
    case Token::Type::TOKEN_ADD:
      return ExpressionOperatorType::ADD;
    case Token::Type::TOKEN_MINUS:
      return ExpressionOperatorType::MINUS;
    case Token::Type::TOKEN_MULTIPLY:
      return ExpressionOperatorType::MULTIPLY;
    case Token::Type::TOKEN_DIVIDE:
      return ExpressionOperatorType::DIVIDE;
    case Token::Type::TOKEN_EQUALS:
      return ExpressionOperatorType::EQUALS;
    case Token::Type::TOKEN_NOT_EQUALS:
      return ExpressionOperatorType::NOT_EQUALS;
    case Token::Type::TOKEN_LOGICAL_AND:
      return ExpressionOperatorType::LOGICAL_AND;
    case Token::Type::TOKEN_LOGICAL_OR:
      return ExpressionOperatorType::LOGICAL_OR;
    case Token::Type::TOKEN_LOGICAL_NOT:
      return ExpressionOperatorType::LOGICAL_NOT;
    case Token::Type::TOKEN_LESS_THAN:
      return ExpressionOperatorType::LESS_THAN;
    case Token::Type::TOKEN_LESS_THAN_OR_EQUAL:
      return ExpressionOperatorType::LESS_THAN_OR_EQUAL;
    case Token::Type::TOKEN_GREATER_THAN:
      return ExpressionOperatorType::GREATER_THAN;
    case Token::Type::TOKEN_GREATER_THAN_OR_EQUAL:
      return ExpressionOperatorType::GREATER_THAN_OR_EQUAL;
    default:
      std::cout << token->lexerContext << " Parser: Token (" << *token << ") is not an op token." << std::endl;
      throw std::exception();
  }
}
