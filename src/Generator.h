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

std::string registerToByteEquivalent(Register reg, unsigned int bytes);
std::string registerTo8BitEquivalent(Register reg);
std::string registerTo16BitEquivalent(Register reg);
std::string registerTo32BitEquivalent(Register reg);

struct OutputFile {
  std::string filepath;
  std::ofstream* fileStream;
};

struct BlockScope {
  struct Variable {
    unsigned int offset;
    Location* location;
    DataType dataType;
  };
  std::map<std::string, Variable> variables;
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

  std::stack<BlockScope> blockScopeStack;

public:
  Generator(ASTNode* astRoot, std::string filepath);
  ~Generator();

  void generate();
  void walkBlock(ASTBlock* node, bool isEntrypoint = false);
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

  void swapLocation(Register reg, Location* oldLoc, Location* newLoc);
  void removeLocation(Register reg, Location* oldLoc);
  void moveToRegister(Register reg, Location* location);
  Register getAvailableRegister();
  Register getRegisterForConst(Location* location, unsigned long long constant);
  Register getRegisterForConst(Location* location, const std::string& constant);
  Register getRegisterFor(Location* location, bool isConstant = false, const std::string& constant = "");
  void moveToMem(const std::string& ident, Location* loc, unsigned int bytes);
  Location* recallFromMem(const std::string& ident, unsigned int bytes);

  void comment(const std::string& comment);
};


#endif //COMPILER_VISUALIZATION_GENERATOR_H
