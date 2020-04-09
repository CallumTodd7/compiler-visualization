//
// Created by Callum Todd on 2020/03/23.
//

#ifndef COMPILER_VISUALIZATION_GENERATOR_H
#define COMPILER_VISUALIZATION_GENERATOR_H

#include "AST.h"
#include <fstream>
#include <map>

enum Register {
  NONE = 1000,
  RAX = 0,
  RBX = 1,
  RCX = 2,
  RDX = 3,
  RSI = 4,
  RDI = 5,
};
#define TOTAL_REGISTERS 6
std::ostream& operator<<(std::ostream& os, const Register& reg);

struct OutputFile {
  std::string filepath;
  std::ofstream* fileStream;
};

class Generator {
private:
  ASTNode* astRoot;
  OutputFile* file;

  bool genComments = true;

  std::map<std::string, std::string> constants;
  unsigned int constantIndex = 0;

  Location* registerContents[TOTAL_REGISTERS];
  std::map<Location*, Register> locationMap;

public:
  Generator(ASTNode* astRoot, std::string filepath);
  ~Generator();

  void generate();
  void walkBlock(ASTBlock* node);
  void walkStatement(ASTStatement* node);
  Location* walkExpression(ASTExpression* node);
  void walkVariableDeclaration(ASTVariableDeclaration* node);
  void walkProcedureDeclaration(ASTProcedure* node);
  void walkProcedureCall(ASTProcedureCall* node);
  void walkVariableAssignment(ASTVariableAssignment* node);
  void walkIf(ASTIf* node);
  void walkWhile(ASTWhile* node);
  void walkContinue(ASTContinue* node);
  void walkBreak(ASTBreak* node);
  void walkReturn(ASTReturn* node);
  Location* walkBinOp(ASTBinOp* node);
  Location* walkUnaryOp(ASTUnaryOp* node);
  Location* walkLiteral(ASTLiteral* node);
  Location* walkVariableIdent(ASTVariableIdent* node);

  void writeStringLiteralList(const std::string& str);
  void pushCallerSaved();
  void popCallerSaved();
  Register getRegisterForConst(Location* location, unsigned long long constant);
  Register getRegisterForConst(Location* location, const std::string& constant);
  Register getRegisterFor(Location* location, bool isConstant = false, const std::string& constant = "");
  void moveToRegister(Register reg, Location* location);
  Register getAvailableRegister();

  void comment(const std::string& comment);
};


#endif //COMPILER_VISUALIZATION_GENERATOR_H
