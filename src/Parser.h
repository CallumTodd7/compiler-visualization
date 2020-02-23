//
// Created by Callum Todd on 2020/02/21.
//

#ifndef COMPILER_VISUALIZATION_PARSER_H
#define COMPILER_VISUALIZATION_PARSER_H


#include <vector>
#include "AST.h"
#include "Lexer.h"

class Parser {
private:
  std::vector<Token> tokenStream;
  unsigned long tokenStreamIndex = 0;

  std::stack<ASTBlock*> blockScopeStack;

public:
  explicit Parser(std::vector<Token> tokenStream);

  ASTBlock* parse();

private:
  Token* currentToken();
  Token* nextToken();
  Token* peekToken();

  const Token* expect(Token::Type type);
  const Token* accept(Token::Type type);

  ASTBlock* parseBlock();
  ASTStatement* parseStatement();
  ASTProcedure* parseProcedureDeclaration();
  ASTVariableDeclaration* parseVariableDeclaration(bool declaratorOnly = false);
  ASTVariableAssignment* parseVariableAssignment();
  ASTProcedureCall* parseProcedureCall();
  ASTWhile* parseWhile();
  ASTIf* parseIf();
  ASTContinue* parseContinue();
  ASTBreak* parseBreak();
  ASTReturn* parseReturn();
  ASTExpression* parseExpression();

  DataType parseTypeSpecifier();
};


#endif //COMPILER_VISUALIZATION_PARSER_H
