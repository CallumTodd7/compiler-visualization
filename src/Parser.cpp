//
// Created by Callum Todd on 2020/02/21.
//

#include <iostream>
#include "Parser.h"


Parser::Parser(std::vector<Token> tokenStream)
  : tokenStream(std::move(tokenStream)) {
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
  const Token* token = currentToken();
  if (token && token->type == type) {
    return nextToken();
  }

  std::cout << token->lexerContext
    << " Parser: Unexpected token. Got " << token->type << ", expected " << type << "." << std::endl;
  throw std::exception();
}

const Token* Parser::accept(Token::Type type) {
  const Token* token = currentToken();
  if (token && token->type == type) {
    return nextToken();
  }

  return nullptr;
}

ASTBlock* Parser::parse() {
  auto node = new ASTBlock();

  node->parent = nullptr;

  blockScopeStack.push(node);

  while (tokenStreamIndex < tokenStream.size()) {
    node->statements.push_back(parseProcedureDeclaration());
  }

  blockScopeStack.pop();

  return node;
}

ASTBlock* Parser::parseBlock() {
  auto node = new ASTBlock();

  node->parent = blockScopeStack.top();

  expect(Token::Type::TOKEN_BRACE_OPEN);
  blockScopeStack.push(node);

  while (!accept(Token::Type::TOKEN_BRACE_CLOSE)) {
    node->statements.push_back(parseStatement());
  }

  blockScopeStack.pop();

  return node;
}

ASTProcedure* Parser::parseProcedureDeclaration() {
  auto node = new ASTProcedure();

  node->returnType = parseTypeSpecifier();

  //TODO replace with proc call
  const Token* procIdentToken = expect(Token::Type::TOKEN_IDENTIFIER);
  node->ident = std::string(procIdentToken->value.stringValue.data, procIdentToken->value.stringValue.count);//TODO use atom

  expect(Token::Type::TOKEN_PARENTHESIS_OPEN);

  while (!accept(Token::Type::TOKEN_PARENTHESIS_CLOSE)) {
    node->parameters.push_back(parseVariableDeclaration(true));
    if (peekToken()->type != Token::Type::TOKEN_PARENTHESIS_CLOSE) {
      expect(Token::Type::TOKEN_COMMA);
    }
  }

  node->block = parseBlock();

  return node;
}

ASTVariableDeclaration* Parser::parseVariableDeclaration(bool declaratorOnly) {
  auto node = new ASTVariableDeclaration();

  node->dataType = parseTypeSpecifier();

  //TODO replace with proc call
  const Token* procIdentToken = expect(Token::Type::TOKEN_IDENTIFIER);
  node->ident = std::string(procIdentToken->value.stringValue.data, procIdentToken->value.stringValue.count);//TODO use atom

  if (!declaratorOnly) {
    if (accept(Token::Type::TOKEN_EQUALS)) {
      node->initalValueExpression = parseExpression();
    }

    expect(Token::Type::TOKEN_SEMICOLON);
  }

  return node;
}

ASTVariableAssignment* Parser::parseVariableAssignment() {
  auto node = new ASTVariableAssignment();

  //TODO replace with proc call
  const Token* procIdentToken = expect(Token::Type::TOKEN_IDENTIFIER);
  node->ident = std::string(procIdentToken->value.stringValue.data, procIdentToken->value.stringValue.count);//TODO use atom

  expect(Token::Type::TOKEN_EQUALS);

  node->newValueExpression = parseExpression();

  expect(Token::Type::TOKEN_SEMICOLON);

  return node;
}

ASTProcedureCall* Parser::parseProcedureCall() {
  auto node = new ASTProcedureCall();

  //TODO replace with proc call
  const Token* procIdentToken = expect(Token::Type::TOKEN_IDENTIFIER);
  node->ident = std::string(procIdentToken->value.stringValue.data, procIdentToken->value.stringValue.count);//TODO use atom

  expect(Token::Type::TOKEN_PARENTHESIS_OPEN);

  while (!accept(Token::Type::TOKEN_PARENTHESIS_CLOSE)) {
    node->parameters.push_back(parseExpression());
    if (peekToken()->type != Token::Type::TOKEN_PARENTHESIS_CLOSE) {
      expect(Token::Type::TOKEN_COMMA);
    }
  }

  expect(Token::Type::TOKEN_SEMICOLON);

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

  expect(Token::Type::TOKEN_KEYWORD_WHILE);

  expect(Token::Type::TOKEN_PARENTHESIS_OPEN);
  node->conditional = parseExpression();
  expect(Token::Type::TOKEN_PARENTHESIS_CLOSE);

  node->body = parseBlock();

  return node;
}

ASTIf* Parser::parseIf() {
  auto node = new ASTIf();

  expect(Token::Type::TOKEN_KEYWORD_IF);

  expect(Token::Type::TOKEN_PARENTHESIS_OPEN);
  node->conditional = parseExpression();
  expect(Token::Type::TOKEN_PARENTHESIS_CLOSE);

  node->trueStatement = parseBlock();

  if (accept(Token::Type::TOKEN_KEYWORD_ELSE)) {
    if (peekToken()->type == Token::Type::TOKEN_BRACE_OPEN) {
      node->falseStatement = parseBlock();
    } else {
      node->falseStatement = parseIf();
    }
  } else {
    node->falseStatement = nullptr;
  }

  return node;
}

ASTContinue* Parser::parseContinue() {
  auto node = new ASTContinue();

  expect(Token::Type::TOKEN_KEYWORD_CONTINUE);
  expect(Token::Type::TOKEN_SEMICOLON);

  return node;
}

ASTBreak* Parser::parseBreak() {
  auto node = new ASTBreak();

  expect(Token::Type::TOKEN_KEYWORD_BREAK);
  expect(Token::Type::TOKEN_SEMICOLON);

  return node;
}

ASTReturn* Parser::parseReturn() {
  auto node = new ASTReturn();

  expect(Token::Type::TOKEN_KEYWORD_RETURN);
  node->expression = parseExpression();
  expect(Token::Type::TOKEN_SEMICOLON);

  return node;
}

ASTExpression* Parser::parseExpression() {
  //TODO do more than literals
  auto node = new ASTLiteral();

  const Token* token = expect(Token::Type::TOKEN_INTEGER);
  node->valueType = ASTLiteral::ValueType::INTEGER;
  node->value.integerData = token->value.integerValue;

  return node;
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
