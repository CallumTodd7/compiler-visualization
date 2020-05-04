//
// Created by Callum Todd on 2020/02/15.
//

#include "Lexer.h"
#include "AST.h"
#include "Generator.h"

std::ostream& operator<<(std::ostream& os, const Token::Type& type) {
  switch (type) {
    case Token::Type::TOKEN_ERROR:
      os << "ERROR";
      break;
    case Token::Type::TOKEN_EOF:
      os << "EOF";
      break;
    case Token::Type::TOKEN_IDENTIFIER:
      os << "IDENTIFIER";
      break;
    case Token::Type::TOKEN_STRING:
      os << "STRING";
      break;
    case Token::Type::TOKEN_INTEGER:
      os << "INTEGER";
      break;
    case Token::Type::TOKEN_SEMICOLON:
      os << "SEMICOLON";
      break;
    case Token::Type::TOKEN_COLON:
      os << "COLON";
      break;
    case Token::Type::TOKEN_ADD:
      os << "ADD";
      break;
    case Token::Type::TOKEN_MINUS:
      os << "MINUS";
      break;
    case Token::Type::TOKEN_MULTIPLY:
      os << "MULTIPLY";
      break;
    case Token::Type::TOKEN_DIVIDE:
      os << "DIVIDE";
      break;
    case Token::Type::TOKEN_PARENTHESIS_OPEN:
      os << "PARENTHESIS_OPEN";
      break;
    case Token::Type::TOKEN_PARENTHESIS_CLOSE:
      os << "PARENTHESIS_CLOSE";
      break;
    case Token::Type::TOKEN_BRACE_OPEN:
      os << "BRACE_OPEN";
      break;
    case Token::Type::TOKEN_BRACE_CLOSE:
      os << "BRACE_CLOSE";
      break;
    case Token::Type::TOKEN_ASSIGN:
      os << "ASSIGN";
      break;
    case Token::Type::TOKEN_EQUALS:
      os << "EQUALS";
      break;
    case Token::Type::TOKEN_NOT_EQUALS:
      os << "NOT_EQUALS";
      break;
    case Token::Type::TOKEN_LOGICAL_AND:
      os << "LOGICAL_AND";
      break;
    case Token::Type::TOKEN_LOGICAL_OR:
      os << "LOGICAL_OR";
      break;
    case Token::Type::TOKEN_LOGICAL_NOT:
      os << "LOGICAL_NOT";
      break;
    case Token::Type::TOKEN_LESS_THAN:
      os << "LESS_THAN";
      break;
    case Token::Type::TOKEN_LESS_THAN_OR_EQUAL:
      os << "LESS_THAN_OR_EQUAL";
      break;
    case Token::Type::TOKEN_GREATER_THAN:
      os << "GREATER_THAN";
      break;
    case Token::Type::TOKEN_GREATER_THAN_OR_EQUAL:
      os << "GREATER_THAN_OR_EQUAL";
      break;
    case Token::Type::TOKEN_COMMA:
      os << "COMMA";
      break;
    case Token::Type::TOKEN_DOT:
      os << "DOT";
      break;
    case Token::Type::TOKEN_KEYWORD_U8:
      os << "KEYWORD_U8";
      break;
    case Token::Type::TOKEN_KEYWORD_S8:
      os << "KEYWORD_S8";
      break;
    case Token::Type::TOKEN_KEYWORD_VOID:
      os << "KEYWORD_VOID";
      break;
    case Token::Type::TOKEN_KEYWORD_RETURN:
      os << "KEYWORD_RETURN";
      break;
    case Token::Type::TOKEN_KEYWORD_CONTINUE:
      os << "KEYWORD_CONTINUE";
      break;
    case Token::Type::TOKEN_KEYWORD_BREAK:
      os << "KEYWORD_BREAK";
      break;
    case Token::Type::TOKEN_KEYWORD_IF:
      os << "KEYWORD_IF";
      break;
    case Token::Type::TOKEN_KEYWORD_ELSE:
      os << "KEYWORD_ELSE";
      break;
    case Token::Type::TOKEN_KEYWORD_WHILE:
      os << "KEYWORD_WHILE";
      break;
    case Token::Type::TOKEN_KEYWORD_FOR:
      os << "KEYWORD_FOR";
      break;
    case Token::Type::TOKEN_KEYWORD_EXTERN:
      os << "KEYWORD_EXTERN";
      break;
    default:
      os << "ERROR_TYPE_HAS_NO_STRING_VALUE";
      break;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Token& token) {
  os << token.lexerContext << " " << token.type;
  switch (token.valueType) {
    case Token::ValueType::NONE:
      break;
    case Token::ValueType::STRING:
      os << "(" << std::string(token.value.stringValue.data, token.value.stringValue.count) << ")";
      break;
    case Token::ValueType::INTEGER:
      os << "(" << token.value.integerValue << ")";
      break;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const LexerContext& context) {
  if (context.file) {
    os << context.file->filepath;
  } else {
    os << "NULL";
  }
  os << ":" << context.lineNumber << ":" << context.characterPos;
  return os;
}

std::ostream& operator<<(std::ostream& os, const ASTType& astType) {
  switch (astType) {
    case UNINITIALISED:
      os << "UNINITIALISED";
      break;
    case BLOCK:
      os << "BLOCK";
      break;
    case PROC_DECL:
      os << "PROC_DECL";
      break;
    case PROC_CALL:
      os << "PROC_CALL";
      break;
    case VARIABLE_DECL:
      os << "VARIABLE_DECL";
      break;
    case VARIABLE_ASSIGNMENT:
      os << "VARIABLE_ASSIGNMENT";
      break;
    case IF:
      os << "IF";
      break;
    case WHILE:
      os << "WHILE";
      break;
    case CONTINUE:
      os << "CONTINUE";
      break;
    case BREAK:
      os << "BREAK";
      break;
    case RETURN:
      os << "RETURN";
      break;
    case BIN_OP:
      os << "BIN_OP";
      break;
    case UNARY_OP:
      os << "UNARY_OP";
      break;
    case LITERAL:
      os << "LITERAL";
      break;
    case VARIABLE:
      os << "VARIABLE";
      break;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const ExpressionOperatorType& opType) {
  switch (opType) {
    case ExpressionOperatorType::UNINITIALISED:
      os << "UNINITIALISED";
      break;
    case ExpressionOperatorType::ADD:
      os << "ADD";
      break;
    case ExpressionOperatorType::MINUS:
      os << "MINUS";
      break;
    case ExpressionOperatorType::MULTIPLY:
      os << "MULTIPLY";
      break;
    case ExpressionOperatorType::DIVIDE:
      os << "DIVIDE";
      break;
    case ExpressionOperatorType::EQUALS:
      os << "EQUALS";
      break;
    case ExpressionOperatorType::NOT_EQUALS:
      os << "NOT_EQUALS";
      break;
    case ExpressionOperatorType::LOGICAL_AND:
      os << "LOGICAL_AND";
      break;
    case ExpressionOperatorType::LOGICAL_OR:
      os << "LOGICAL_OR";
      break;
    case ExpressionOperatorType::LOGICAL_NOT:
      os << "LOGICAL_NOT";
      break;
    case ExpressionOperatorType::LESS_THAN:
      os << "LESS_THAN";
      break;
    case ExpressionOperatorType::LESS_THAN_OR_EQUAL:
      os << "LESS_THAN_OR_EQUAL";
      break;
    case ExpressionOperatorType::GREATER_THAN:
      os << "GREATER_THAN";
      break;
    case ExpressionOperatorType::GREATER_THAN_OR_EQUAL:
      os << "GREATER_THAN_OR_EQUAL";
      break;
  }
  return os;
}
std::ostream& operator<<(std::ostream& os, const DataType& dataType) {
  switch (dataType) {
    case DataType::UNINITIALISED:
      os << "UNINITIALISED";
      break;
    case DataType::VOID:
      os << "VOID";
      break;
    case DataType::U8:
      os << "U8";
      break;
    case DataType::S8:
      os << "S8";
      break;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Register& reg) {
  switch (reg) {
    case NONE:
      os << "REGISTER_NONE";
      break;
    case RAX:
      os << "rax";
      break;
    case RBX:
      os << "rbx";
      break;
    case RCX:
      os << "rcx";
      break;
    case RDX:
      os << "rdx";
      break;
    case RSI:
      os << "rsi";
      break;
    case RDI:
      os << "rdi";
      break;
    case R8:
      os << "r8";
      break;
    case R9:
      os << "r9";
      break;
    case R10:
      os << "r10";
      break;
    case R11:
      os << "r11";
      break;
    case R15:
      os << "r15";
      break;
    default:
      os << "UNKNOWN_REGISTER(" << (int)reg << ")";
      break;
  }
  return os;
}

unsigned int Location::idCount = 0;//TODO FIXME Move to proper file

std::ostream& operator<<(std::ostream& os, const Location& location) {
  os << "loc" << location.id;
  return os;
}

std::ostream& operator<<(std::ostream& os, const Location* location) {
  os << *location;
  return os;
}

std::ostream& operator<<(std::ostream& os, const Label& label) {
  os << "_lbl" << label.id;
  return os;
}

std::ostream& operator<<(std::ostream& os, const ASTNode& node) {
  node.print(os, 0);
  return os;
}

void indent(std::ostream& os, unsigned int level) {
  for (unsigned int i = 0; i < level; ++i) {
    os << "  ";
  }
}

void ASTNode::print(std::ostream& os, unsigned int level) const {
  indent(os, level);
  os << "- " << type;
}

void ASTBlock::print(std::ostream& os, unsigned int level) const {
  ASTNode::print(os, level);
  level++;
  for (auto& statement : this->statements) {
    os << "\n";
    statement->print(os, level);
  }
}

void ASTProcedure::print(std::ostream& os, unsigned int level) const {
  ASTNode::print(os, level);
  os << "(";
  if (this->isExternal) {
    os << "extern: ";
  }
  os << this->ident << ")";

  if (this->block) {
    os << "\n";
    this->block->print(os, ++level);
  }
}

void ASTWhile::print(std::ostream& os, unsigned int level) const {
  ASTNode::print(os, level);
  level++;

  os << "\n";
  indent(os, level);
  os << "Conditional:\n";
  this->conditional->print(os, level + 1);

  os << "\n";
  indent(os, level);
  os << "Body:\n";
  this->body->print(os, level + 1);
}

void ASTVariableDeclaration::print(std::ostream& os, unsigned int level) const {
  ASTNode::print(os, level);
  os << "(" << this->ident << ")";

  os << "\n";
  this->initialValueExpression->print(os, ++level);
}

void ASTVariableAssignment::print(std::ostream& os, unsigned int level) const {
  ASTNode::print(os, level);
  os << "(" << this->ident << ")";

  os << "\n";
  this->newValueExpression->print(os, ++level);
}

void ASTProcedureCall::print(std::ostream& os, unsigned int level) const {
  ASTNode::print(os, level);
  os << "(" << this->ident << ")";
}

void ASTIf::print(std::ostream& os, unsigned int level) const {
  ASTNode::print(os, level);
}

void ASTReturn::print(std::ostream& os, unsigned int level) const {
  ASTNode::print(os, level);

  if (this->expression) {
    os << "\n";
    this->expression->print(os, ++level);
  }
}

void ASTLiteral::print(std::ostream& os, unsigned int level) const {
  ASTNode::print(os, level);
  if (this->valueType == ValueType::INTEGER) {
    os << "(" << this->value.integerData << ")";
  }
}

void ASTBinOp::print(std::ostream& os, unsigned int level) const {
  ASTNode::print(os, level);
  os << "(" << this->op << ")";
  level++;

  os << "\n";
  indent(os, level);
  os << "Left:\n";
  this->left->print(os, level + 1);

  os << "\n";
  indent(os, level);
  os << "Right:\n";
  this->right->print(os, level + 1);
}

void ASTUnaryOp::print(std::ostream& os, unsigned int level) const {
  ASTNode::print(os, level);
  os << "(" << this->op << ")";
  this->child->print(os, ++level);
}

void ASTVariableIdent::print(std::ostream& os, unsigned int level) const {
  ASTNode::print(os, level);
  os << "(" << this->ident << ")";
}
