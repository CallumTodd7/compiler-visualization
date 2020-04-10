//
// Created by Callum Todd on 2020/02/23.
//

#ifndef COMPILER_VISUALIZATION_AST_H
#define COMPILER_VISUALIZATION_AST_H

#include <vector>
#include <string>
#include "Types.h"

struct Location {
  int id;

  Location() : id(idCount++) {}

private:
  static int idCount;

};
std::ostream& operator<<(std::ostream& os, const Location& location);
std::ostream& operator<<(std::ostream& os, const Location* location);

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
  UNARY_OP,
  LITERAL,
  VARIABLE,
};
std::ostream& operator<<(std::ostream& os, const ASTType& type);

struct ASTNode {
  ASTType type = ASTType::UNINITIALISED;

  virtual void print(std::ostream& os, unsigned int level) const;
  friend std::ostream& operator<<(std::ostream& os, const ASTNode& node);
};

struct ASTStatement : ASTNode {
};

struct ASTExpression : ASTNode {
  Location* location = new Location();
};

struct ASTBlock : ASTStatement {
  ASTBlock() { type = ASTType::BLOCK; }
  void print(std::ostream& os, unsigned int level) const override;

  ASTBlock* parent = nullptr;

  std::vector<ASTStatement*> statements;
};

struct ASTVariableDeclaration : ASTStatement {
  ASTVariableDeclaration() { type = ASTType::VARIABLE_DECL; }
  void print(std::ostream& os, unsigned int level) const override;

  DataType dataType = DataType::UNINITIALISED;
  std::string ident;//TODO replace with atom
  ASTExpression* initalValueExpression = nullptr;
};

struct ASTVariableAssignment : ASTStatement {
  ASTVariableAssignment() { type = ASTType::VARIABLE_ASSIGNMENT; }
  void print(std::ostream& os, unsigned int level) const override;

  std::string ident;//TODO replace with atom
  ASTExpression* newValueExpression = nullptr;
};

struct ASTProcedure : ASTStatement {
  ASTProcedure() { type = ASTType::PROC_DECL; }
  void print(std::ostream& os, unsigned int level) const override;

  ASTBlock* parent = nullptr;

  DataType returnType = DataType::VOID;
  std::string ident;//TODO replace with atom

  std::vector<ASTVariableDeclaration*> parameters;

  ASTBlock* block = nullptr;
};

struct ASTProcedureCall : ASTStatement {
  ASTProcedureCall() { type = ASTType::PROC_CALL; }
  void print(std::ostream& os, unsigned int level) const override;

  std::string ident;//TODO replace with atom

  std::vector<ASTExpression*> parameters;
};

struct ASTIf : ASTStatement {
  ASTIf() { type = ASTType::IF; }
  void print(std::ostream& os, unsigned int level) const override;

  ASTExpression* conditional = nullptr;
  ASTStatement* trueStatement = nullptr;
  ASTStatement* falseStatement = nullptr;
};

struct ASTWhile : ASTStatement {
  ASTWhile() { type = ASTType::WHILE; }
  void print(std::ostream& os, unsigned int level) const override;

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
  void print(std::ostream& os, unsigned int level) const override;

  ASTExpression* expression = nullptr;
};

struct ASTLiteral : ASTExpression {
  ASTLiteral() { type = ASTType::LITERAL; }
  void print(std::ostream& os, unsigned int level) const override;

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

enum class ExpressionOperatorType {
  UNINITIALISED = 0,

  ADD,
  MINUS,
  MULTIPLY,
  DIVIDE,
  EQUALS,
  NOT_EQUALS,
  LOGICAL_AND,
  LOGICAL_OR,
  LOGICAL_NOT,
  LESS_THAN,
  LESS_THAN_OR_EQUAL,
  GREATER_THAN,
  GREATER_THAN_OR_EQUAL,
};
std::ostream& operator<<(std::ostream& os, const ExpressionOperatorType& type);

struct ASTBinOp : ASTExpression {
  ASTBinOp() { type = ASTType::BIN_OP; }
  void print(std::ostream& os, unsigned int level) const override;

  ASTExpression* left = nullptr;
  ExpressionOperatorType op = ExpressionOperatorType::UNINITIALISED;
  ASTExpression* right = nullptr;
};

struct ASTUnaryOp : ASTExpression {
  ASTUnaryOp() { type = ASTType::UNARY_OP; }
  void print(std::ostream& os, unsigned int level) const override;

  ASTExpression* child = nullptr;
  ExpressionOperatorType op = ExpressionOperatorType::UNINITIALISED;
};

struct ASTVariableIdent : ASTExpression {
  ASTVariableIdent() { type = ASTType::VARIABLE; }
  void print(std::ostream& os, unsigned int level) const override;

  DataType dataType = DataType::UNINITIALISED;
  std::string ident;//TODO replace with atom
};

#endif //COMPILER_VISUALIZATION_AST_H
