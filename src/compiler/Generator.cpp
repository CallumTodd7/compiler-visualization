//
// Created by Callum Todd on 2020/03/23.
//

#include <iostream>
#include <sstream>
#include <functional>
#include "Generator.h"
#include "AST.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-pro-type-static-cast-downcast"

std::string registerToByteEquivalent(Register reg, unsigned int bytes) {
  if (bytes == 1) {
    return registerTo8BitEquivalent(reg);
  } else if (bytes == 2) {
    return registerTo16BitEquivalent(reg);
  } else if (bytes == 4) {
    return registerTo32BitEquivalent(reg);
  } else if (bytes == 8) {
    std::stringstream ss;
    ss << reg;
    return ss.str();
  } else {
    std::cout << "Byte amount not supported!" << std::endl;
    throw std::exception();
  }
}

std::string registerTo8BitEquivalent(Register reg) {
  switch (reg) {
    case NONE: return "REGISTER_NONE";
    case RAX: return "al";
    case RBX: return "bl";
    case RCX: return "cl";
    case RDX: return "dl";
    case RSI: return "sil";
    case RDI: return "dil";
    case R8:
    case R9:
    case R10:
    case R11:
    case R15: {
      std::stringstream ss;
      ss << reg << "b";
      return ss.str();
    }
  }
  throw std::exception();
}

std::string registerTo16BitEquivalent(Register reg) {
  switch (reg) {
    case NONE: return "REGISTER_NONE";
    case RAX: return "ax";
    case RBX: return "bx";
    case RCX: return "cx";
    case RDX: return "dx";
    case RSI: return "si";
    case RDI: return "di";
    case R8:
    case R9:
    case R10:
    case R11:
    case R15: {
      std::stringstream ss;
      ss << reg << "w";
      return ss.str();
    }
  }
  throw std::exception();
}

std::string registerTo32BitEquivalent(Register reg) {
  switch (reg) {
    case NONE: return "REGISTER_NONE";
    case RAX: return "eax";
    case RBX: return "ebx";
    case RCX: return "ecx";
    case RDX: return "edx";
    case RSI: return "esi";
    case RDI: return "edi";
    case R8:
    case R9:
    case R10:
    case R11:
    case R15: {
      std::stringstream ss;
      ss << reg << "d";
      return ss.str();
    }
  }
  throw std::exception();
}

ParameterClass identifyParamClass(DataType dataType) {
  switch (dataType) {
    case DataType::U8: return INTEGER;
    case DataType::S8: return INTEGER;

    case DataType::UNINITIALISED:
    case DataType::VOID:
      throw std::exception();
  }
}

unsigned int Label::idCount = 1;

Generator::Generator(ASTNode* astRoot, std::string filepath) {
  this->astRoot = astRoot;

  auto outputStream = new std::ofstream(filepath, std::ios::binary);
  this->file = new OutputFile{
      .filepath = std::move(filepath),
      .fileStream = outputStream,
  };
}

Generator::~Generator() {
  file->fileStream->close();
}

void Generator::generate() {
  std::cout << "Generating asm" << std::endl;

  comment("BEGIN leading boilerplate");
  *file->fileStream << "global main\n";
  comment("END leading boilerplate");

  if (astRoot->type == ASTType::BLOCK) {
    auto* block = static_cast<ASTBlock*>(astRoot);

    comment("BEGIN externs");
    for (auto& statement : block->statements) {
      if (statement->type == ASTType::PROC_DECL) {
        auto* procDecl = static_cast<ASTProcedure*>(statement);
        if (procDecl->isExternal) {
          *file->fileStream << "extern " << procDecl->ident << "\n";
        }
      }
    }
    comment("END externs");

    comment("BEGIN program");
    *file->fileStream << "section .text\n";

    walkBlock(block, true);
    comment("END program");
  } else {
    std::cout << "Root is not block. Bad." << std::endl;
    file->fileStream->close();
    throw std::exception();
  }

  comment("BEGIN constants");
  *file->fileStream << "section .data\n";
  for (const auto& constantPair : constants) {
    auto str = constantPair.first;
    auto id = constantPair.second;

    *file->fileStream << id << ": db ";
    writeStringLiteralList(str);
    *file->fileStream << ", 0\n";
  }
  comment("END constants");

  file->fileStream->close();
  std::cout << "Done! See: " << file->filepath << std::endl;
}

void Generator::writeStringLiteralList(const std::string& str) {
  unsigned int charIndex = 0;

  while (charIndex < str.size()) {
    if (charIndex != 0) {
      *file->fileStream << ", ";
    }

    if (str[charIndex] == '\\') {
      if (charIndex + 1 < str.size()) {
        char pc = str[charIndex + 1];

        switch (pc) {
          case 'n':
            *file->fileStream << "10";
            charIndex += 2;
            continue;
          case '0':
            *file->fileStream << "0";
            charIndex += 2;
            continue;
          default: break;
        }
      }
    }

    std::string strSegment;
    while (charIndex < str.size() && str[charIndex] != '\\') {
      strSegment.push_back(str[charIndex]);
      charIndex++;
    }
    *file->fileStream << "\"" << strSegment << "\"";
  }
}

void Generator::walkBlock(ASTBlock* node,
                          bool isEntrypoint,
                          const std::function<void(BlockScope&)>& initCallback) {
  comment("BEGIN Block");

  if (!isEntrypoint) {
    // Set stackbase pointer
    *file->fileStream << "push rbp\n";
    *file->fileStream << "mov rbp, rsp\n";
  }

  // Allocate local var space on stack
  BlockScope scope = BlockScope();
  scope.stackHeight = blockScopeStack.size();
  if (!blockScopeStack.empty()) {
    scope.parent = &blockScopeStack.top();
  }

  for (auto statement : node->statements) {
    if (statement->type == ASTType::VARIABLE_DECL) {
      auto* statementNode = static_cast<ASTVariableDeclaration*>(statement);

      BlockScope::Variable var = BlockScope::Variable();
      var.offset = scope.totalLocalBytes;
      var.location = new Location();
      var.dataType = statementNode->dataType;
      var.stackHeight = scope.stackHeight;
      scope.variables.insert_or_assign(statementNode->ident, var);

      scope.totalLocalBytes += bytesOf(statementNode->dataType);
    }
  }

  if (initCallback) {
    initCallback(scope);
  }

  if (scope.totalLocalBytes) {
    *file->fileStream << "sub rsp, " << scope.totalLocalBytes << "\n";
  }

  blockScopeStack.push(scope);

  // Perform block
  for (auto statement : node->statements) {
    walkStatement(statement);
  }

  // Cleanup

  blockScopeStack.pop();

  if (!isEntrypoint) {
    // Reset stackbase pointer & Dealloc local var space on stack
    *file->fileStream << "mov rsp, rbp\n";
    *file->fileStream << "pop rbp\n";
  } else {
    // Dealloc local var space on stack
    *file->fileStream << "add rsp, " << scope.totalLocalBytes << "\n";
  }

  comment("END Block");
}

void Generator::walkStatement(ASTStatement* node) {
  switch (node->type) {
    case UNINITIALISED:
      std::cout << "Statement uninitialised" << std::endl;
      file->fileStream->close();
      throw std::exception();
    case BLOCK:
      walkBlock(static_cast<ASTBlock*>(node));
      break;
    case PROC_DECL:
      walkProcedureDeclaration(static_cast<ASTProcedure*>(node));
      break;
    case PROC_CALL:
      walkProcedureCall(static_cast<ASTProcedureCall*>(node));
      break;
    case VARIABLE_DECL:
      walkVariableDeclaration(static_cast<ASTVariableDeclaration*>(node));
      break;
    case VARIABLE_ASSIGNMENT:
      walkVariableAssignment(static_cast<ASTVariableAssignment*>(node));
      break;
    case IF:
      walkIf(static_cast<ASTIf*>(node));
      break;
    case WHILE:
      walkWhile(static_cast<ASTWhile*>(node));
      break;
    case CONTINUE:
      walkContinue(static_cast<ASTContinue*>(node));
      break;
    case BREAK:
      walkBreak(static_cast<ASTBreak*>(node));
      break;
    case RETURN:
      walkReturn(static_cast<ASTReturn*>(node));
      break;
    default:
      std::cout << "Statement must be an expression. Bad." << std::endl;
      file->fileStream->close();
      throw std::exception();
  }
}

Location* Generator::walkExpression(ASTExpression* node) {
  switch (node->type) {
    case BIN_OP:
      return walkBinOp(static_cast<ASTBinOp*>(node));
    case UNARY_OP:
      return walkUnaryOp(static_cast<ASTUnaryOp*>(node));
    case LITERAL:
      return walkLiteral(static_cast<ASTLiteral*>(node));
    case VARIABLE:
      return walkVariableIdent(static_cast<ASTVariableIdent*>(node));
    default:
      std::cout << "Expression must be a statement. Bad." << std::endl;
      file->fileStream->close();
      throw std::exception();
  }
}

void Generator::walkVariableDeclaration(ASTVariableDeclaration* node) {
  comment("BEGIN VariableDeclaration");

  Location* loc = walkExpression(node->initalValueExpression);
  moveToMem(node->ident, loc, bytesOf(node->dataType));

  removeLocation(loc);

  comment("END VariableDeclaration");
}

void Generator::walkProcedureDeclaration(ASTProcedure* node) {
  comment("BEGIN ProcedureDeclaration");

  if (node->isExternal) {
    comment("external: " + node->ident);

    // Add procedure to block's scope
    BlockScope::Procedure proc = BlockScope::Procedure();

    unsigned int totalParamsInRegisters = 0;
    for (auto parameter : node->parameters) {
      BlockScope::Procedure::Parameter* param = new BlockScope::Procedure::Parameter(); //@LEAK
      param->dataType = parameter->dataType;

      if (totalParamsInRegisters < TOTAL_PROC_CALL_REGISTERS) {
        // Enough registers
        param->paramClass = ParameterClass::INTEGER;//TODO = identifyParamClass(param->dataType);
      } else {
        // Fallback to stack
        param->paramClass = ParameterClass::MEMORY;
      }

      switch (param->paramClass) {
        case MEMORY: {
          // Use stack
          break;
        }
        case INTEGER: {
          // Use register
          param->paramRegister = procCallRegisterOrder[totalParamsInRegisters++];
          break;
        }
        default:
          std::cout << "Not implemented" << std::endl;
          throw std::exception();
      }

      proc.parameters.push_back(param);
    }

    blockScopeStack.top().procedures.insert_or_assign(node->ident, proc);

  } else {

    std::vector<Location*> parameterLocations;

    // Write head
    *file->fileStream << node->ident << ":\n";

    // Write block
    walkBlock(node->block, false, [&](BlockScope& scope) -> void {
      scope.isProcedureBlock = true;
      // TODO set scope.endLabel (required for `return`)
      // What do we do for procs with return data types?

      // Add procedure to block's scope
      BlockScope::Procedure proc = BlockScope::Procedure();

      // Add parameters to block's scope
      unsigned int totalParamStackBytes = 0;
      unsigned int totalParamsInRegisters = 0;
      for (auto parameter : node->parameters) {
        BlockScope::Variable var = BlockScope::Variable();
        BlockScope::Procedure::Parameter* param = new BlockScope::Procedure::Parameter(); //@LEAK

        var.parameter = param;
        var.location = new Location();
        var.location->isParameter = true;
        var.dataType = parameter->dataType;
        var.stackHeight = scope.stackHeight;

        param->dataType = parameter->dataType;

        if (totalParamsInRegisters < TOTAL_PROC_CALL_REGISTERS) {
          // Enough registers
          param->paramClass = ParameterClass::INTEGER;//TODO = identifyParamClass(param->dataType);
        } else {
          // Fallback to stack
          param->paramClass = ParameterClass::MEMORY;
        }

        switch (param->paramClass) {
          case MEMORY: {
            // Use stack
            var.offset = totalParamStackBytes;
            totalParamStackBytes += bytesOf(parameter->dataType);
            break;
          }
          case INTEGER: {
            // Use register
            param->paramRegister = procCallRegisterOrder[totalParamsInRegisters++];

            registerContents[param->paramRegister] = var.location;
            locationMap.insert_or_assign(var.location, param->paramRegister);
            break;
          }
          default:
            std::cout << "Not implemented" << std::endl;
            throw std::exception();
        }

        proc.parameters.push_back(param);
        scope.variables.insert_or_assign(parameter->ident, var);

        parameterLocations.push_back(var.location);
      }

      if (scope.parent) {
        scope.parent->procedures.insert_or_assign(node->ident, proc);
      } else {
        std::cout << "Can't add procedure" << std::endl;
        throw std::exception();
      }
    });

    // Write tail
    *file->fileStream << "ret\n";

    // Remove params from internal map
    for (auto* loc : parameterLocations) {
      removeLocation(loc, true);
    }

  }

  comment("END ProcedureDeclaration");
}

void Generator::walkProcedureCall(ASTProcedureCall* node) {
  comment("BEGIN ProcedureCall");

  auto proc = blockScopeStack.top().searchForProcedure(node->ident);

  // TODO expand check for matching data types too (i.e. Check the proc signature)
  if (proc.parameters.size() != node->parameters.size()) {
    std::cout << "Parameter mismatch! "
              << "Proc decl has " << proc.parameters.size() << ", "
              << "proc call has " << node->parameters.size() << std::endl;
    throw std::exception();
  }

  std::vector<Location*> paramLocs;
  std::vector<Register> requiredRegistersForParams;

  // Always used for varargs count
  requiredRegistersForParams.push_back(Register::RAX);

  for (size_t i = 0; i < node->parameters.size(); ++i) {
    paramLocs.push_back(walkExpression(node->parameters[i]));

    switch (proc.parameters[i]->paramClass) {
      case MEMORY:
        break;
      case INTEGER:
        if (requiredRegistersForParams.size() <= TOTAL_PROC_CALL_REGISTERS) {
          requiredRegistersForParams.push_back(proc.parameters[i]->paramRegister);
        }
        break;
      default:
        std::cout << "Not implemented" << std::endl;
        throw std::exception();
    }
  }

//  requireRegistersFree(requiredRegistersForParams);//TODO USE

  unsigned int registersUsed = 0;
  for (size_t i = 0; i < node->parameters.size(); ++i) {
    switch (proc.parameters[i]->paramClass) {
      case MEMORY: {
        // Use stack
        std::cout << "Not implemented yet" << std::endl;
        file->fileStream->close();
        throw std::exception();
      }
      case INTEGER: {
        // Use register
        Register reg = procCallRegisterOrder[registersUsed++];
        moveToRegister(reg, paramLocs[i], requiredRegistersForParams);
        break;
      }
      default:
        std::cout << "Not implemented" << std::endl;
        throw std::exception();
    }
  }
  *file->fileStream << "xor rax, rax\n"; //zero rax because printf is varargs & no floats

  auto savedRegisters = pushCallerSaved();

  *file->fileStream << "call " << node->ident << "\n";

  for (size_t i = 0; i < paramLocs.size(); ++i) {
    if (i < TOTAL_PROC_CALL_REGISTERS) {
      removeLocation(procCallRegisterOrder[i], nullptr, true);
    } else {
      std::cout << "Not implemented yet" << std::endl;
      file->fileStream->close();
      throw std::exception();
    }
  }

  popCallerSaved(savedRegisters);

  // Remove params from internal map
  for (auto* loc : paramLocs) {
    removeLocation(loc);
  }

  comment("END ProcedureCall");
}

void Generator::walkVariableAssignment(ASTVariableAssignment* node) {
  comment("BEGIN VariableAssignment");

  auto var = blockScopeStack.top().searchForVariable(node->ident);
  Location* loc = walkExpression(node->newValueExpression);
  moveToMem(node->ident, loc, bytesOf(var.dataType));

  removeLocation(loc);

  comment("END VariableAssignment");
}

void Generator::walkIf(ASTIf* node) {
  comment("BEGIN If");

  Label labelFalse(node->falseStatement), labelEnd;

  // Condition
  walkExpression(node->conditional);
  auto* tempLoc = new Location();
  Register regConditionalResult = getRegisterForCopy(node->conditional->location, tempLoc);

  *file->fileStream << "test " << regConditionalResult << ", " << regConditionalResult << "\n";
  removeLocation(regConditionalResult, tempLoc);
  removeLocation(node->conditional->location, false);

  if (node->falseStatement) {
    *file->fileStream << "jz " << labelFalse << "\n";
  } else {
    *file->fileStream << "jz " << labelEnd << "\n";
  }

  // True
  walkStatement(node->trueStatement);

  // False
  if (node->falseStatement) {
    *file->fileStream << "jmp " << labelEnd << "\n";
    *file->fileStream << labelFalse << ":\n";

    walkStatement(node->falseStatement);
  }

  // End
  *file->fileStream << labelEnd << ":\n";

  comment("END If");
}

void Generator::walkWhile(ASTWhile* node) {
  comment("BEGIN While");

  Label labelStart, labelEnd;

  // Condition
  *file->fileStream << labelStart << ":\n";

  walkExpression(node->conditional);
  auto* tempLoc = new Location();
  Register regConditionalResult = getRegisterForCopy(node->conditional->location, tempLoc);

  *file->fileStream << "test " << regConditionalResult << ", " << regConditionalResult << "\n";
  removeLocation(regConditionalResult, tempLoc);
  removeLocation(node->conditional->location, false);

  *file->fileStream << "jz " << labelEnd << "\n";

  // Body
  walkBlock(node->body, false, [labelStart, labelEnd](BlockScope& scope) -> void {
    scope.startLabel = labelStart;
    scope.endLabel = labelEnd;
  });

  *file->fileStream << "jmp " << labelStart << "\n";

  // End
  *file->fileStream << labelEnd << ":\n";

  comment("END While");
}

void Generator::walkContinue(ASTContinue* node) {
  comment("BEGIN Continue");

  BlockScope* scope = &blockScopeStack.top();
  while (scope) {
    if (scope->isProcedureBlock) {
      break;
    }
    if (scope->startLabel.exists()) {
      // Cleanly end blocks before jump
      unsigned int stackHeightDist = blockScopeStack.top().stackHeight - scope->stackHeight + 1;
      for (unsigned int i = 0; i < stackHeightDist; ++i) {
        // Reset stackbase pointer & Dealloc local var space on stack
        *file->fileStream << "mov rsp, rbp\n";
        *file->fileStream << "pop rbp\n";
      }

      // Jump
      *file->fileStream << "jmp " << scope->startLabel << "\n";
      break;
    } else {
      scope = scope->parent;
    }
  }

  comment("END Continue");
}

void Generator::walkBreak(ASTBreak* node) {
  comment("BEGIN Break");

  BlockScope* scope = &blockScopeStack.top();
  while (scope) {
    if (scope->isProcedureBlock) {
      break;
    }
    if (scope->endLabel.exists()) {
      // Cleanly end blocks before jump
      unsigned int stackHeightDist = blockScopeStack.top().stackHeight - scope->stackHeight + 1;
      for (unsigned int i = 0; i < stackHeightDist; ++i) {
        // Reset stackbase pointer & Dealloc local var space on stack
        *file->fileStream << "mov rsp, rbp\n";
        *file->fileStream << "pop rbp\n";
      }

      // Jump
      *file->fileStream << "jmp " << scope->endLabel << "\n";
      break;
    } else {
      scope = scope->parent;
    }
  }

  comment("END Break");
}

void Generator::walkReturn(ASTReturn* node) {
  comment("BEGIN Return");

  BlockScope* scope = &blockScopeStack.top();
  while (scope) {
    if (scope->isProcedureBlock) {
      if (scope->endLabel.exists()) {
        // Cleanly end blocks before jump
        unsigned int stackHeightDist = blockScopeStack.top().stackHeight - scope->stackHeight + 1;
        for (unsigned int i = 0; i < stackHeightDist; ++i) {
          // Reset stackbase pointer & Dealloc local var space on stack
          *file->fileStream << "mov rsp, rbp\n";
          *file->fileStream << "pop rbp\n";
        }

        // Jump
        *file->fileStream << "jmp " << scope->endLabel << "\n";
        break;
      }
      break;
    }
    scope = scope->parent;
  }

  comment("END Return");
}

Location* Generator::walkBinOp(ASTBinOp* node) {
  comment("BEGIN BinOp");
  //DEFER: comment("END BinOp");

  // Special case for Logical Or: Don't walk right side if left is true
  if (node->op == ExpressionOperatorType::LOGICAL_OR) {
    walkExpression(node->left);
    auto* tempLeftLoc = new Location();
    Register regLeft = getRegisterForCopy(node->left->location, tempLeftLoc);

    walkExpression(node->right);
    Register regRight = getRegisterFor(node->right->location);

    removeLocation(node->left->location, false);

    std::cout << "LOGICAL_OR not implemented" << std::endl;
    file->fileStream->close();
    throw std::exception();

    comment("END BinOp");
    return node->location;
  }

  walkExpression(node->left);
  walkExpression(node->right);
  auto* tempLeftLoc = new Location();
  Register regLeft = getRegisterForCopy(node->left->location, tempLeftLoc);
  Register regRight = getRegisterFor(node->right->location);

  switch (node->op) {
    case ExpressionOperatorType::ADD:
      *file->fileStream << "add " << regLeft << ", " << regRight << "\n";
      swapLocation(regLeft, tempLeftLoc, node->location);
      removeLocation(regRight, node->right->location);
      break;
    case ExpressionOperatorType::MINUS:
      *file->fileStream << "sub " << regLeft << ", " << regRight << "\n";
      swapLocation(regLeft, tempLeftLoc, node->location);
      removeLocation(regRight, node->right->location);
      break;
    case ExpressionOperatorType::MULTIPLY: {
      *file->fileStream << "imul " << regLeft << ", " << regRight << "\n";
      swapLocation(regLeft, tempLeftLoc, node->location);
      removeLocation(regRight, node->right->location);
      break;
    }
    case ExpressionOperatorType::DIVIDE: {
      *file->fileStream << "idiv " << regLeft << ", " << regRight << "\n";
      swapLocation(regLeft, tempLeftLoc, node->location);
      removeLocation(regRight, node->right->location);
      break;
    }
    case ExpressionOperatorType::EQUALS: {
      *file->fileStream << "cmp " << regLeft << ", " << regRight << "\n";
      std::string regLeft8BitStr = registerTo8BitEquivalent(regLeft);
      *file->fileStream << "setz " << regLeft8BitStr << "\n";
      *file->fileStream << "movzx " << regLeft << ", " << regLeft8BitStr << "\n";
      swapLocation(regLeft, tempLeftLoc, node->location);
      removeLocation(regRight, node->right->location);
      break;
    }
    case ExpressionOperatorType::NOT_EQUALS: {
      *file->fileStream << "cmp " << regLeft << ", " << regRight << "\n";
      std::string regLeft8BitStr = registerTo8BitEquivalent(regLeft);
      *file->fileStream << "setnz " << regLeft8BitStr << "\n";
      *file->fileStream << "movzx " << regLeft << ", " << regLeft8BitStr << "\n";
      swapLocation(regLeft, tempLeftLoc, node->location);
      removeLocation(regRight, node->right->location);
      break;
    }
    case ExpressionOperatorType::LESS_THAN: {
      *file->fileStream << "cmp " << regLeft << ", " << regRight << "\n";
      std::string regLeft8BitStr = registerTo8BitEquivalent(regLeft);
      *file->fileStream << "setl " << regLeft8BitStr << "\n";
      *file->fileStream << "movzx " << regLeft << ", " << regLeft8BitStr << "\n";
      swapLocation(regLeft, tempLeftLoc, node->location);
      removeLocation(regRight, node->right->location);
      break;
    }
    case ExpressionOperatorType::LESS_THAN_OR_EQUAL: {
      *file->fileStream << "cmp " << regLeft << ", " << regRight << "\n";
      std::string regLeft8BitStr = registerTo8BitEquivalent(regLeft);
      *file->fileStream << "setle " << regLeft8BitStr << "\n";
      *file->fileStream << "movzx " << regLeft << ", " << regLeft8BitStr << "\n";
      swapLocation(regLeft, tempLeftLoc, node->location);
      removeLocation(regRight, node->right->location);
      break;
    }
    case ExpressionOperatorType::GREATER_THAN: {
      *file->fileStream << "cmp " << regLeft << ", " << regRight << "\n";
      std::string regLeft8BitStr = registerTo8BitEquivalent(regLeft);
      *file->fileStream << "setg " << regLeft8BitStr << "\n";
      *file->fileStream << "movzx " << regLeft << ", " << regLeft8BitStr << "\n";
      swapLocation(regLeft, tempLeftLoc, node->location);
      removeLocation(regRight, node->right->location);
      break;
    }
    case ExpressionOperatorType::GREATER_THAN_OR_EQUAL: {
      *file->fileStream << "cmp " << regLeft << ", " << regRight << "\n";
      std::string regLeft8BitStr = registerTo8BitEquivalent(regLeft);
      *file->fileStream << "setge " << regLeft8BitStr << "\n";
      *file->fileStream << "movzx " << regLeft << ", " << regLeft8BitStr << "\n";
      swapLocation(regLeft, tempLeftLoc, node->location);
      removeLocation(regRight, node->right->location);
      break;
    }
    case ExpressionOperatorType::LOGICAL_AND: {
      std::cout << "LOGICAL_AND not implemented" << std::endl;
      file->fileStream->close();
      throw std::exception();
    }
    case ExpressionOperatorType::UNINITIALISED:
      std::cout << "BinOp UNINITIALISED" << std::endl;
      file->fileStream->close();
      throw std::exception();
    default:
      std::cout << "BinOp not implemented: " << node->op << std::endl;
      file->fileStream->close();
      throw std::exception();
  }

  removeLocation(node->left->location, false);

  comment("END BinOp");

  return node->location;
}

Location* Generator::walkUnaryOp(ASTUnaryOp* node) {
  comment("BEGIN UnaryOp");

  walkExpression(node->child);
  auto* tempChildLoc = new Location();
  Register regChild = getRegisterForCopy(node->child->location, tempChildLoc);

  switch (node->op) {
    case ExpressionOperatorType::ADD:
      swapLocation(regChild, tempChildLoc, node->location);
      break;
    case ExpressionOperatorType::MINUS:
      *file->fileStream << "xor " << Register::R15 << ", " << Register::R15 << ";zero\n";
      *file->fileStream << "sub " << Register::R15 << ", " << regChild << "\n";
      *file->fileStream << "mov " << regChild << ", " << Register::R15 << "\n";
      swapLocation(regChild, tempChildLoc, node->location);
      break;
    case ExpressionOperatorType::LOGICAL_NOT: {
      *file->fileStream << "cmp " << regChild << ", " << 0 << "\n";
      std::string regChild8BitStr = registerTo8BitEquivalent(regChild);
      *file->fileStream << "sete " << regChild8BitStr << "\n";
      *file->fileStream << "movzx " << regChild << ", " << regChild8BitStr << "\n";
      swapLocation(regChild, tempChildLoc, node->location);
      break;
    }
    case ExpressionOperatorType::UNINITIALISED:
      std::cout << "UnaryOp UNINITIALISED" << std::endl;
      file->fileStream->close();
      throw std::exception();
    default:
      std::cout << "UnaryOp not implemented: " << node->op << std::endl;
      file->fileStream->close();
      throw std::exception();
  }

  removeLocation(node->child->location, false);

  comment("END UnaryOp");

  return node->location;
}

Location* Generator::walkLiteral(ASTLiteral* node) {
  comment("BEGIN Literal");

  switch (node->valueType) {
    case ASTLiteral::ValueType::STRING: {
      std::string str = std::string(node->value.stringData.data, node->value.stringData.count);
      std::string id = "ds" + std::to_string(constantIndex++);
      auto ret = constants.insert({str, id});
      if (ret.second == false) {
        id = ret.first->second;
      }
      getRegisterForConst(node->location, id);
//      *file->fileStream << "mov rdi, " << id << "\n";
      break;
    }
    case ASTLiteral::ValueType::INTEGER: {
      getRegisterForConst(node->location, node->value.integerData);
      break;
    }
    default:
    case ASTLiteral::ValueType::NONE:
      break;
  }

  comment("END Literal");

  return node->location;
}

Location* Generator::walkVariableIdent(ASTVariableIdent* node) {
  comment("BEGIN VariableIdent");

  auto var = blockScopeStack.top().searchForVariable(node->ident);
  if (node->dataType == DataType::UNINITIALISED) {
    node->dataType = var.dataType;
  }

  if (var.parameter) {
    switch (var.parameter->paramClass) {
      case MEMORY: { // Use stack
        node->location = recallFromMem(node->ident, bytesOf(node->dataType));
        break;
      }
      case INTEGER: { // Use register
        node->location = var.location;
        break;
      }
      default:
        std::cout << "Not implemented" << std::endl;
        throw std::exception();
    }
  } else {
    node->location = recallFromMem(node->ident, bytesOf(node->dataType));
  }

  comment("END VariableIdent");

  return node->location;
}

std::vector<std::pair<Register, Location*>> Generator::pushCallerSaved() {
  comment("BEGIN pushCallerSaved");

  std::vector<std::pair<Register, Location*>> savedRegisters;

  for (int i = 0; i < TOTAL_REGISTERS; ++i) {
    auto reg = (Register) i;

    // TODO if 'reg' is in 'calleSavedRegister', then 'continue'.
    // This is because calle saved registers must be
    // preserved in all functions and won't be over ridden

    Location* loc = registerContents[reg];
    if (loc != nullptr) {
      *file->fileStream << "push " << reg;
      if (genComments) {
        *file->fileStream << "; with " << loc;
      }
      *file->fileStream << "\n";
      savedRegisters.emplace_back(reg, loc);
    }
  }

  comment("END pushCallerSaved");

  return savedRegisters;
}

void Generator::popCallerSaved(std::vector<std::pair<Register, Location*>> savedRegisters) {
  comment("BEGIN popCallerSaved");

  for (size_t i = savedRegisters.size() - 1; savedRegisters.size() > i; --i) {
    Register reg = savedRegisters[i].first;
    Location* loc = savedRegisters[i].second;

    *file->fileStream << "pop " << reg;
    if (genComments) {
      *file->fileStream << "; with " << loc;
    }
    *file->fileStream << "\n";


    swapLocation(reg, registerContents[reg], loc);
  }

  comment("END popCallerSaved");
}

Register Generator::registerWithAddressForVariable(BlockScope::Variable variable) {
  Register reg = Register::R15;

  unsigned int stackHeightDist = blockScopeStack.top().stackHeight - variable.stackHeight;
  unsigned int offset = variable.offset;

#ifndef NDEBUG
  dumpRegisters(true);
#endif

  if (stackHeightDist > 0) {
    *file->fileStream << "mov " << reg << ", [rbp]\n";
    for (unsigned int i = 0; i < stackHeightDist - 1; ++i) {
      *file->fileStream << "mov " << reg << ", [" << reg << "]\n";
    }
  } else {
    *file->fileStream << "mov " << reg << ", rbp\n";
  }

  if (variable.parameter) {
    // Skips over procedure's return address and stackbase pointer on the stack
    constexpr unsigned int parameterOffset = 8;

    *file->fileStream << "add " << reg << ", " << (parameterOffset + offset) << "\n";
  } else {
    offset += 1; // + 1 because we're using rbp instead of rsp
    *file->fileStream << "sub " << reg << ", " << offset << "\n";
  }

  return reg;
}

void Generator::moveToMem(const std::string& ident, Location* location, unsigned int bytes) {
  BlockScope::Variable var = blockScopeStack.top().searchForVariable(ident);

  Register locRegister = locationMap.at(location);
  std::string locRegisterStr = registerToByteEquivalent(locRegister, bytes);
  if (genComments) {
    *file->fileStream << ";mov " << *location << " to mem(ident: " << ident << ")\n";
  }
  Register varRegister = registerWithAddressForVariable(var);
  *file->fileStream << "mov [" << varRegister << "], " << locRegisterStr << "\n";
}

Location* Generator::recallFromMem(const std::string& ident, unsigned int bytes) {
  BlockScope::Variable var = blockScopeStack.top().searchForVariable(ident);

#ifndef NDEBUG
  dumpRegisters(true);
#endif

  Register varRegister = registerWithAddressForVariable(var);

  Register locRegister = getAvailableRegister();
  std::string locRegisterStr = registerToByteEquivalent(locRegister, bytes);
  if (bytes != 8) {
    // Zero 64bit register if only a part of it will be filled
    *file->fileStream << "xor " << locRegister << ", " << locRegister << "\n";
  }
  *file->fileStream << "mov " << locRegisterStr << ", [" << varRegister << "]\n";

  registerContents[locRegister] = var.location;
  locationMap.insert_or_assign(var.location, locRegister);

  return var.location;
}

void Generator::recallFromParamRegister(Location* location) {
  Register newRegister = getAvailableRegister();

  Register exitingRegister = locationMap.at(location);

  if (genComments) {
    *file->fileStream << ";mov " << location << " (in " << exitingRegister << ") to "
      << newRegister << "\n";
  }
  *file->fileStream << "mov " << newRegister << ", " << exitingRegister << "\n";

  registerContents[newRegister] = location;
  locationMap.insert_or_assign(location, newRegister);
}

void Generator::requireRegistersFree(const std::vector<Register>& registers) {
  for (auto reg : registers) {
    // If register is not empty
    if (registerContents[reg] != nullptr) {
      Register newReg = getAvailableRegister(registers);
      moveToRegister(newReg, registerContents[reg]);
    }
  }
}

void Generator::moveToRegister(Register newRegister,
                               Location* location,
                               const std::vector<Register>& excludingRegisters) {
  if (registerContents[newRegister] == location) {
    // Already there; do nothing
    return;
  }

#ifndef NDEBUG
  dumpRegisters(true);
#endif

  if (registerContents[newRegister] != nullptr) {
    std::cout << "WARNING: Register " << newRegister
              << " already has contents (" << registerContents[newRegister]
              << ") when asked to make way for " << *location
              << " (Relocating)" << std::endl;

    Register freeRegister = getAvailableRegister(excludingRegisters);
    Location* existingLoc = registerContents[newRegister];

    if (genComments) {
      *file->fileStream << ";mov " << *existingLoc << " to " << freeRegister
                        << " (relocation)" << "\n";
    }
    *file->fileStream << "mov " << freeRegister << ", " << newRegister << "\n";

    registerContents[freeRegister] = existingLoc;
    locationMap.insert_or_assign(existingLoc, freeRegister);
  }

  Register oldRegister = locationMap.at(location);
  if (genComments) {
    *file->fileStream << ";mov " << *location << " to " << newRegister << "\n";
  }
  *file->fileStream << "mov " << newRegister << ", " << oldRegister << "\n";

  registerContents[oldRegister] = nullptr;
  registerContents[newRegister] = location;
  locationMap.insert_or_assign(location, newRegister);
}

void Generator::swapLocation(Register reg, Location* oldLoc, Location* newLoc, bool forceRmParam) {
  registerContents[reg] = newLoc;

  if (oldLoc && (!oldLoc->isParameter || forceRmParam)) {
    locationMap.erase(oldLoc);
  }
  locationMap.insert_or_assign(newLoc, reg);
}

void Generator::removeLocation(Register reg, Location* oldLoc, bool forceRmParam) {
  Location* existingLoc = registerContents[reg];

  if (!forceRmParam) {
    if (oldLoc) {
      if (oldLoc->isParameter) return;
    } else {
      if (existingLoc->isParameter) return;
    }
  }

  registerContents[reg] = nullptr;

  if (oldLoc) {
    locationMap.erase(oldLoc);
  } else {
    locationMap.erase(existingLoc);
  }
}

void Generator::removeLocation(Location* oldLoc, bool forceRmParam) {
  if (oldLoc->isParameter && !forceRmParam) {
    return;
  }

  Register reg;
  try {
    reg = locationMap.at(oldLoc);
  } catch (std::out_of_range&) {
    return;
  }

  registerContents[reg] = nullptr;
  locationMap.erase(oldLoc);
}

Register Generator::getAvailableRegister() {
  Register availableRegister = Register::NONE;

  for (int i = 0; i < TOTAL_REGISTERS; ++i) {
    auto reg = (Register) i;
    Location* loc = registerContents[i];

    if (availableRegister == Register::NONE && loc == nullptr) {
      availableRegister = reg;
    }
  }

  if (availableRegister == Register::NONE) {
    std::cout << "No available register!" << std::endl;
    file->fileStream->close();
    throw std::exception();
  }

  return availableRegister;
}

Register Generator::getAvailableRegister(const std::vector<Register>& excludingRegisters) {
  Register availableRegister = Register::NONE;

  for (int i = 0; i < TOTAL_REGISTERS; ++i) {
    auto reg = (Register) i;

    auto it = std::find(excludingRegisters.begin(), excludingRegisters.end(), reg);
    if (it != excludingRegisters.end()) {
      continue;
    }

    Location* loc = registerContents[i];

    if (availableRegister == Register::NONE && loc == nullptr) {
      availableRegister = reg;
    }
  }

  if (availableRegister == Register::NONE) {
    std::cout << "No available register!" << std::endl;
    file->fileStream->close();
    throw std::exception();
  }

  return availableRegister;
}

Register Generator::getRegisterForConst(Location* location, unsigned long long constant) {
  std::stringstream ss;
  ss << constant;
  return getRegisterFor(location, true, ss.str());
}

Register Generator::getRegisterForConst(Location* location, const std::string& constant) {
  return getRegisterFor(location, true, constant);
}

Register Generator::getRegisterFor(Location* location, bool isConstant, const std::string& constant) {
  Register availableRegister = Register::NONE;

  // Already exists?
  for (int i = 0; i < TOTAL_REGISTERS; ++i) {
    auto reg = (Register) i;
    Location* loc = registerContents[i];

    // Return register
    if (location != nullptr && location == loc) {
      return reg;
    }

    if (availableRegister == Register::NONE && loc == nullptr) {
      availableRegister = reg;
    }
  }

  // No, then we'll put it in an available one
  if (availableRegister != Register::NONE) {
#ifndef NDEBUG
    dumpRegisters(true);
#endif

    if (isConstant) {
      *file->fileStream << "mov " << availableRegister << ", " << constant << "\n";
      registerContents[availableRegister] = location;
      locationMap.insert_or_assign(location, availableRegister);
    } else {
      moveToRegister(availableRegister, location);
    }
    return availableRegister;
  }

  // Else, error (for now; TODO)
  std::cout << "No register for " << *location << std::endl;
  file->fileStream->close();
  throw std::exception();
}

Register Generator::getRegisterForCopy(Location* location, Location* newLocation) {
  Register newRegister = getAvailableRegister();

#ifndef NDEBUG
  dumpRegisters(true);
#endif

  Register oldRegister = locationMap.at(location);
  if (genComments) {
    *file->fileStream << ";mov " << *location << " to " << newRegister << "\n";
  }
  *file->fileStream << "mov " << newRegister << ", " << oldRegister << "\n";

  registerContents[newRegister] = location;
  locationMap.insert_or_assign(newLocation, newRegister);

  return newRegister;
}

void Generator::comment(const std::string& comment) {
  if (!genComments) return;

  *file->fileStream << ";" << comment << "\n";
}

void Generator::dumpRegisters(bool mismatchCheckOnly) {
  int regCount = 0;
  int locCount = 0;

  // Count registers
  for (int i = 0; i < TOTAL_REGISTERS; ++i) {
    auto reg = (Register)i;
    Location* loc = registerContents[reg];
    if (loc) {
      regCount++;
    }
  }

  // Count locations
  for (auto pair : locationMap) {
    Register reg = pair.second;
    if (reg != Register::NONE) {
      locCount++;
    }
  }

  // Return if no mismatch
  if (mismatchCheckOnly && locCount == regCount) {
    return;
  }

  // Print registers
  if (genComments) {
    *file->fileStream << ";Dump registers:\n";
  }
  std::cout << "Dump registers:" << std::endl;
  for (int i = 0; i < TOTAL_REGISTERS; ++i) {
    auto reg = (Register)i;
    Location* loc = registerContents[reg];
    if (loc) {
      if (genComments) {
        *file->fileStream << "; - " << reg << ": " << loc->id << "\n";
      }
      std::cout << "- " << reg << ": " << loc->id << std::endl;
    } else {
      if (genComments) {
        *file->fileStream << "; - " << reg << ": -" << "\n";
      }
      std::cout << "- " << reg << ": -" << std::endl;
    }
  }

  // Print locations
  if (genComments) {
    *file->fileStream << ";Dump locations:\n";
  }
  std::cout << "Dump locations:" << std::endl;
  for (auto pair : locationMap) {
    Location* loc = pair.first;
    Register reg = pair.second;
    if (reg != Register::NONE) {
      if (genComments) {
        *file->fileStream << "; - " << loc->id << ": " << reg << "\n";
      }
      std::cout << "- " << loc->id << ": " << reg << std::endl;
    } else {
      if (genComments) {
        *file->fileStream << "; - " << loc->id << ": -" << "\n";
      }
      std::cout << "- " << loc->id << ": -" << std::endl;
    }
  }

  // Print mismatch
  if (locCount != regCount) {
    if (genComments) {
      *file->fileStream << "; !!!MISMATCH!!!\n";
    }
    std::cout << "!!!MISMATCH!!!" << std::endl;
  }
}

#pragma clang diagnostic pop