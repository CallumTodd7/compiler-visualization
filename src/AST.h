//
// Created by Callum Todd on 2020/02/23.
//

#ifndef COMPILER_VISUALIZATION_AST_H
#define COMPILER_VISUALIZATION_AST_H

#include <vector>
#include "Types.h"

enum ASTType {
  UNINITIALISED = 0,

  // Statements

  BLOCK,
  PROC_DECL,
  PROC_CALL,
  VARIABLE_DECL,
  VARIABLE_ASSIGNMENT,
  IF,
  WHILE,
  CONTINUE,
  BREAK,
  RETURN,

  // Expressions

  BIN_OP,
  LITERAL,
  VARIABLE,
};

struct ASTNode {
  ASTType type = ASTType::UNINITIALISED;
};

struct ASTStatement : ASTNode {
};

struct ASTExpression : ASTNode {
};

struct ASTBlock : ASTStatement {
  ASTBlock() { type = ASTType::BLOCK; }

  ASTBlock* parent = nullptr;

  std::vector<ASTStatement*> statements;
};

struct ASTVariableDeclaration : ASTStatement {
  ASTVariableDeclaration() { type = ASTType::VARIABLE_DECL; }

  DataType dataType = DataType::UNINITIALISED;
  std::string ident;//TODO replace with atom
  ASTExpression* initalValueExpression = nullptr;
};

struct ASTVariableAssignment : ASTStatement {
  ASTVariableAssignment() { type = ASTType::VARIABLE_ASSIGNMENT; }

  std::string ident;//TODO replace with atom
  ASTExpression* newValueExpression = nullptr;
};

struct ASTProcedure : ASTStatement {
  ASTProcedure() { type = ASTType::PROC_DECL; }

  ASTBlock* parent = nullptr;

  DataType returnType = DataType::VOID;
  std::string ident;//TODO replace with atom

  std::vector<ASTVariableDeclaration*> parameters;

  ASTBlock* block = nullptr;
};

struct ASTProcedureCall : ASTStatement {
  ASTProcedureCall() { type = ASTType::PROC_CALL; }

  std::string ident;//TODO replace with atom

  std::vector<ASTExpression*> parameters;
};

struct ASTIf : ASTStatement {
  ASTIf() { type = ASTType::IF; }

  ASTExpression* conditional = nullptr;
  ASTStatement* trueStatement = nullptr;
  ASTStatement* falseStatement = nullptr;
};

struct ASTWhile : ASTStatement {
  ASTWhile() { type = ASTType::WHILE; }

  ASTExpression* conditional = nullptr;
  ASTStatement* body = nullptr;
};

struct ASTContinue : ASTStatement {
  ASTContinue() { type = ASTType::CONTINUE; }
};

struct ASTBreak : ASTStatement {
  ASTBreak() { type = ASTType::BREAK; }
};

struct ASTReturn : ASTStatement {
  ASTReturn() { type = ASTType::RETURN; }

  ASTExpression* expression = nullptr;
};

struct ASTLiteral : ASTExpression {
  ASTLiteral() { type = ASTType::LITERAL; }

  enum class ValueType {
    NONE = 0,
    STRING,
    INTEGER,
  };
  ValueType valueType = ValueType::NONE;
  union {
    unsigned long long integerData;
    struct {
      int count;
      char* data;
    } stringData{};
  } value;
};

#endif //COMPILER_VISUALIZATION_AST_H
