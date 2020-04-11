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
  R8 = 4,
  R9 = 5,
  R10 = 6,
  R11 = 7,
  RSI = 8,
  RDI = 9,

  R15 = 15,
};
#define TOTAL_REGISTERS 10
std::ostream& operator<<(std::ostream& os, const Register& reg);

#define TOTAL_PROC_CALL_REGISTERS 6
constexpr static Register procCallRegisterOrder[] = {
    Register::RDI,
    Register::RSI,
    Register::RDX,
    Register::RCX,
    Register::R8,
    Register::R9,
};
#define TOTAL_PROC_CALL_RETURN_REGISTERS 2
constexpr static Register procCallReturnRegisterOrder[] = {
    Register::RAX,
    Register::RDX,
};
#define TOTAL_CALLE_SAVED_REGISTERS 2
constexpr static Register calleSavedRegisters[] = { // Must be preserved across proc calls
    Register::RBX,
    //Register::R12, Register::R13, Register::R14,
    Register::R15,
    //Register::RSP, Register::RBP,
};

std::string registerToByteEquivalent(Register reg, unsigned int bytes);
std::string registerTo8BitEquivalent(Register reg);
std::string registerTo16BitEquivalent(Register reg);
std::string registerTo32BitEquivalent(Register reg);

enum ParameterClass {
  NO_CLASS = 0,
  MEMORY,
  INTEGER,
  SSE,
  SSEUP,
  X87,
  X87UP,
  COMPLEX_X87,
};
ParameterClass identifyParamClass(DataType dataType);

struct OutputFile {
  std::string filepath;
  std::ofstream* fileStream;
};

struct BlockScope {
  struct Procedure {
    struct Parameter {
      DataType dataType;
      ParameterClass paramClass = ParameterClass::NO_CLASS;
      Register paramRegister = Register::NONE;
    };

    std::vector<Parameter*> parameters;
    DataType returnDataType;
  };
  struct Variable {
    unsigned int stackHeight;
    unsigned int offset = 0;
    Location* location;
    DataType dataType;

    Procedure::Parameter* parameter = nullptr;
  };

  BlockScope* parent = nullptr;
  unsigned int stackHeight;

  unsigned int totalLocalBytes = 0;

  std::map<std::string, Variable> variables;
  std::map<std::string, Procedure> procedures;

  Variable searchForVariable(const std::string& ident) {
    try {
      return variables.at(ident);
    } catch (std::out_of_range&) {
      if (parent) {
        return parent->searchForVariable(ident);
      } else {
        throw std::out_of_range("Variable " + ident + " not found!");
      }
    }
  }

  Procedure searchForProcedure(const std::string& ident) {
    try {
      return procedures.at(ident);
    } catch (std::out_of_range&) {
      if (parent) {
        return parent->searchForProcedure(ident);
      } else {
        throw std::out_of_range("Procedure " + ident + " not found!");
      }
    }
  }
};

class Generator {
private:
  ASTNode* astRoot;
  OutputFile* file;

  bool genComments = true;

  std::map<std::string, std::string> constants;
  unsigned int constantIndex = 0;

  Location* registerContents[TOTAL_REGISTERS] = { nullptr };
  std::map<Location*, Register> locationMap;

  std::stack<BlockScope> blockScopeStack;

public:
  Generator(ASTNode* astRoot, std::string filepath);
  ~Generator();

  void generate();
  void walkBlock(ASTBlock* node, bool isEntrypoint = false, const std::function<void(BlockScope&)>& initCallback = nullptr);
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
  std::vector<std::pair<Register, Location*>> pushCallerSaved();
  void popCallerSaved(std::vector<std::pair<Register, Location*>> savedRegisters);

  void swapLocation(Register reg, Location* oldLoc, Location* newLoc);
  void removeLocation(Register reg, Location* oldLoc = nullptr);
  void removeLocation(Location* oldLoc);
  void requireRegistersFree(const std::vector<Register>& registers);
  void moveToRegister(Register reg, Location* location, const std::vector<Register>& excludingRegisters = {});
  Register getAvailableRegister();
  Register getAvailableRegister(const std::vector<Register>& excludingRegisters);
  Register getRegisterForConst(Location* location, unsigned long long constant);
  Register getRegisterForConst(Location* location, const std::string& constant);
  Register getRegisterFor(Location* location, bool isConstant = false, const std::string& constant = "");

  Register registerWithAddressForVariable(BlockScope::Variable variable);
  void moveToMem(const std::string& ident, Location* loc, unsigned int bytes);
  Location* recallFromMem(const std::string& ident, unsigned int bytes);
  void recallFromParamRegister(Location* location);

  void comment(const std::string& comment);
  void dumpRegisters();
};


#endif //COMPILER_VISUALIZATION_GENERATOR_H
