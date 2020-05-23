//
// Created by Callum Todd on 2020/03/23.
//

#include <iostream>
#include <functional>
#include "Generator.h"
#include "AST.h"
#include "../Data.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-pro-type-static-cast-downcast"

template<typename T>
Generator::StreamHelper& operator<<(Generator::StreamHelper& h, T const& t) {
  *h._fileOut << t;
  h.ss << t;
  return h;
}

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
    case NONE:
      return "REGISTER_NONE";
    case RAX:
      return "al";
    case RBX:
      return "bl";
    case RCX:
      return "cl";
    case RDX:
      return "dl";
    case RSI:
      return "sil";
    case RDI:
      return "dil";
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
    case NONE:
      return "REGISTER_NONE";
    case RAX:
      return "ax";
    case RBX:
      return "bx";
    case RCX:
      return "cx";
    case RDX:
      return "dx";
    case RSI:
      return "si";
    case RDI:
      return "di";
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
    case NONE:
      return "REGISTER_NONE";
    case RAX:
      return "eax";
    case RBX:
      return "ebx";
    case RCX:
      return "ecx";
    case RDX:
      return "edx";
    case RSI:
      return "esi";
    case RDI:
      return "edi";
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
    case DataType::INT:
      return INTEGER;

    case DataType::UNINITIALISED:
    case DataType::VOID:
      throw std::exception();
  }
}

unsigned int Label::idCount = 1;

Generator::Generator(const std::function<void(const Data&)>& ready, ASTNode* astRoot, std::string filepath)
    : ready(ready) {
  this->astRoot = astRoot;

  auto outputStream = new std::ofstream(filepath, std::ios::binary);
  this->file = new OutputFile{
      .filepath = std::move(filepath),
      .fileStream = outputStream,
  };

  _output = StreamHelper(file->fileStream);
}

Generator::~Generator() {
  file->fileStream->close();
}

void Generator::generate() {
  std::cout << "Generating asm" << std::endl;

  comment("BEGIN leading boilerplate");
  output() << "global main\n";
  comment("END leading boilerplate");

  if (astRoot->type == ASTType::BLOCK) {
    auto* block = static_cast<ASTBlock*>(astRoot);

    comment("BEGIN externs");
    for (auto& statement : block->statements) {
      if (statement->type == ASTType::PROC_DECL) {
        auto* procDecl = static_cast<ASTProcedure*>(statement);
        if (procDecl->isExternal) {
          output() << "extern " << procDecl->ident << "\n";
        }
      }
    }
    comment("END externs");

    comment("BEGIN program");
    output() << "section .text\n";

    walkBlock(block, true);
    comment("END program");
  } else {
    std::stringstream ssError;
    ssError << "Root is not block. Bad.";
    std::cout << ssError.str() << std::endl;
    ready({
              .mode = Data::Mode::ERROR,
              .type = Data::Type::MODE_CHANGE,
              .string = ssError.str(),
          });
    file->fileStream->close();
    throw std::exception();
  }

  comment("BEGIN constants");
  output() << "section .data\n";
  for (const auto& constantPair : constants) {
    auto str = constantPair.first;
    auto id = constantPair.second;

    ready({
              .mode = Data::Mode::CODE_GEN,
              .type = Data::Type::SPECIFIC,
              .codeGenState = Data::CodeGenState::POP_CONSTANT,
              .id = id,
          });

    output() << id << ": db ";
    writeStringLiteralList(str);
    output() << ", 0\n";
  }
  comment("END constants");

  flushOutput();
  file->fileStream->close();
  std::cout << "Done! See: " << file->filepath << std::endl;
}

void Generator::writeStringLiteralList(const std::string& str) {
  unsigned int charIndex = 0;

  while (charIndex < str.size()) {
    if (charIndex != 0) {
      output() << ", ";
    }

    if (str[charIndex] == '\\') {
      if (charIndex + 1 < str.size()) {
        char pc = str[charIndex + 1];

        switch (pc) {
          case 'n':
            output() << "10";
            charIndex += 2;
            continue;
          case '0':
            output() << "0";
            charIndex += 2;
            continue;
          default:
            break;
        }
      }
    }

    std::string strSegment;
    while (charIndex < str.size() && str[charIndex] != '\\') {
      strSegment.push_back(str[charIndex]);
      charIndex++;
    }
    output() << "\"" << strSegment << "\"";
  }
}

void Generator::walkBlock(ASTBlock* node,
                          bool isEntrypoint,
                          const std::function<void(BlockScope&)>& initCallback) {
  enterNode(node, "Block");

  if (!isEntrypoint) {
    // Set stackbase pointer
    output() << "push rbp\n";
    output() << "mov rbp, rsp\n";
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
    output() << "sub rsp, " << scope.totalLocalBytes << "\n";
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
    output() << "mov rsp, rbp\n";
    output() << "pop rbp\n";
  } else {
    // Dealloc local var space on stack
    output() << "add rsp, " << scope.totalLocalBytes << "\n";
  }

  exitNode(node, "Block");
}

void Generator::walkStatement(ASTStatement* node) {
  switch (node->type) {
    case UNINITIALISED: {
      std::stringstream ssError;
      ssError << "Statement uninitialised";
      std::cout << ssError.str() << std::endl;
      ready({
                .mode = Data::Mode::ERROR,
                .type = Data::Type::MODE_CHANGE,
                .string = ssError.str(),
            });
      file->fileStream->close();
      throw std::exception();
    }
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
      std::stringstream ssError;
      ssError << "Statement must be an expression. Bad.";
      std::cout << ssError.str() << std::endl;
      ready({
                .mode = Data::Mode::ERROR,
                .type = Data::Type::MODE_CHANGE,
                .string = ssError.str(),
            });
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
      std::stringstream ssError;
      ssError << "Expression must be a statement. Bad.";
      std::cout << ssError.str() << std::endl;
      ready({
                .mode = Data::Mode::ERROR,
                .type = Data::Type::MODE_CHANGE,
                .string = ssError.str(),
            });
      file->fileStream->close();
      throw std::exception();
  }
}

void Generator::walkVariableDeclaration(ASTVariableDeclaration* node) {
  enterNode(node, "VariableDeclaration");

  Location* loc = walkExpression(node->initialValueExpression);
  moveToMem(node->ident, loc, bytesOf(node->dataType));

  removeLocation(loc);

  exitNode(node, "VariableDeclaration");
}

void Generator::walkProcedureDeclaration(ASTProcedure* node) {
  enterNode(node, "ProcedureDeclaration");

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
    output() << node->ident << ":\n";

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
            ready({
                      .mode = Data::Mode::CODE_GEN,
                      .type = Data::Type::SPECIFIC,
                      .codeGenState = Data::CodeGenState::SET_REG_AND_LOC,
                      .reg = param->paramRegister,
                      .loc = var.location,
                  });
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
        std::stringstream ssError;
        ssError << "Can't add procedure";
        std::cout << ssError.str() << std::endl;
        ready({
                  .mode = Data::Mode::ERROR,
                  .type = Data::Type::MODE_CHANGE,
                  .string = ssError.str(),
              });
        throw std::exception();
      }
    });

    // Write tail
    if (node->ident == "main") {
      comment("BEGIN main exit boilerplate");
      output() << "mov rax, 1\n"
             << "xor rdi, rdi\n"
             << "syscall\n";
      comment("END main exit boilerplate");
    } else {
      output() << "ret\n";
    }

    // Remove params from internal map
    for (auto* loc : parameterLocations) {
      removeLocation(loc, true);
    }

  }

  exitNode(node, "ProcedureDeclaration");
}

void Generator::walkProcedureCall(ASTProcedureCall* node) {
  enterNode(node, "ProcedureCall");

  auto proc = blockScopeStack.top().searchForProcedure(node->ident);

  // TODO expand check for matching data types too (i.e. Check the proc signature)
  if (proc.parameters.size() != node->parameters.size()) {
    std::stringstream ssError;
    ssError << "Parameter mismatch! "
            << "Proc decl has " << proc.parameters.size() << ", "
            << "proc call has " << node->parameters.size();
    std::cout << ssError.str() << std::endl;
    ready({
              .mode = Data::Mode::ERROR,
              .type = Data::Type::MODE_CHANGE,
              .string = ssError.str(),
          });
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
  output() << "xor rax, rax\n"; //zero rax because printf is varargs & no floats

  auto savedRegisters = pushCallerSaved();

  output() << "call " << node->ident << "\n";

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

  exitNode(node, "ProcedureCall");
}

void Generator::walkVariableAssignment(ASTVariableAssignment* node) {
  enterNode(node, "VariableAssignment");

  auto var = blockScopeStack.top().searchForVariable(node->ident);
  Location* loc = walkExpression(node->newValueExpression);
  moveToMem(node->ident, loc, bytesOf(var.dataType));

  removeLocation(loc);

  exitNode(node, "VariableAssignment");
}

void Generator::walkIf(ASTIf* node) {
  enterNode(node, "If");

  Label labelFalse(node->falseStatement), labelEnd;

  // Condition
  walkExpression(node->conditional);
  auto* tempLoc = new Location();
  Register regConditionalResult = getRegisterForCopy(node->conditional->location, tempLoc);

  output() << "test " << regConditionalResult << ", " << regConditionalResult << "\n";
  removeLocation(regConditionalResult, tempLoc);
  removeLocation(node->conditional->location, false);

  if (node->falseStatement) {
    output() << "jz " << labelFalse << "\n";
  } else {
    output() << "jz " << labelEnd << "\n";
  }

  // True
  walkStatement(node->trueStatement);

  // False
  if (node->falseStatement) {
    output() << "jmp " << labelEnd << "\n";
    output() << labelFalse << ":\n";

    walkStatement(node->falseStatement);
  }

  // End
  output() << labelEnd << ":\n";

  exitNode(node, "If");
}

void Generator::walkWhile(ASTWhile* node) {
  enterNode(node, "While");

  Label labelStart, labelEnd;

  // Condition
  output() << labelStart << ":\n";

  walkExpression(node->conditional);
  auto* tempLoc = new Location();
  Register regConditionalResult = getRegisterForCopy(node->conditional->location, tempLoc);

  output() << "test " << regConditionalResult << ", " << regConditionalResult << "\n";
  removeLocation(regConditionalResult, tempLoc);
  removeLocation(node->conditional->location, false);

  output() << "jz " << labelEnd << "\n";

  // Body
  walkBlock(node->body, false, [labelStart, labelEnd](BlockScope& scope) -> void {
    scope.startLabel = labelStart;
    scope.endLabel = labelEnd;
  });

  output() << "jmp " << labelStart << "\n";

  // End
  output() << labelEnd << ":\n";

  exitNode(node, "While");
}

void Generator::walkContinue(ASTContinue* node) {
  enterNode(node, "Continue");

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
        output() << "mov rsp, rbp\n";
        output() << "pop rbp\n";
      }

      // Jump
      output() << "jmp " << scope->startLabel << "\n";
      break;
    } else {
      scope = scope->parent;
    }
  }

  exitNode(node, "Continue");
}

void Generator::walkBreak(ASTBreak* node) {
  enterNode(node, "Break");

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
        output() << "mov rsp, rbp\n";
        output() << "pop rbp\n";
      }

      // Jump
      output() << "jmp " << scope->endLabel << "\n";
      break;
    } else {
      scope = scope->parent;
    }
  }

  exitNode(node, "Break");
}

void Generator::walkReturn(ASTReturn* node) {
  enterNode(node, "Return");

  BlockScope* scope = &blockScopeStack.top();
  while (scope) {
    if (scope->isProcedureBlock) {
      if (scope->endLabel.exists()) {
        // Cleanly end blocks before jump
        unsigned int stackHeightDist = blockScopeStack.top().stackHeight - scope->stackHeight + 1;
        for (unsigned int i = 0; i < stackHeightDist; ++i) {
          // Reset stackbase pointer & Dealloc local var space on stack
          output() << "mov rsp, rbp\n";
          output() << "pop rbp\n";
        }

        // Jump
        output() << "jmp " << scope->endLabel << "\n";
        break;
      }
      break;
    }
    scope = scope->parent;
  }

  exitNode(node, "Return");
}

Location* Generator::walkBinOp(ASTBinOp* node) {
  enterNode(node, "BinOp");
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

    exitNode(node, "BinOp");
    return node->location;
  }

  walkExpression(node->left);
  walkExpression(node->right);
  auto* tempLeftLoc = new Location();
  Register regLeft = getRegisterForCopy(node->left->location, tempLeftLoc);
  Register regRight = getRegisterFor(node->right->location);

  switch (node->op) {
    case ExpressionOperatorType::ADD:
      output() << "add " << regLeft << ", " << regRight << "\n";
      swapLocation(regLeft, tempLeftLoc, node->location);
      removeLocation(regRight, node->right->location);
      break;
    case ExpressionOperatorType::MINUS:
      output() << "sub " << regLeft << ", " << regRight << "\n";
      swapLocation(regLeft, tempLeftLoc, node->location);
      removeLocation(regRight, node->right->location);
      break;
    case ExpressionOperatorType::MULTIPLY: {
      output() << "imul " << regLeft << ", " << regRight << "\n";
      swapLocation(regLeft, tempLeftLoc, node->location);
      removeLocation(regRight, node->right->location);
      break;
    }
    case ExpressionOperatorType::DIVIDE: {
      // Move in dividend
      moveToRegister(Register::RAX, tempLeftLoc);

      // Zero result for remainder
      requireRegistersFree({Register::RDX});
      output() << "xor " << Register::RDX << ", " << Register::RDX << " ";
      comment("zero");

      // Sign extend above two
      output() << "cdq ";
      comment("sign extend EAX into EDX");

      // Divide by divisor
      output() << "idiv " << registerTo32BitEquivalent(regRight) << "\n";

      // Extract quotient
      swapLocation(Register::RAX, tempLeftLoc, node->location);
      removeLocation(regRight, node->right->location);
      break;
    }
    case ExpressionOperatorType::EQUALS: {
      output() << "cmp " << regLeft << ", " << regRight << "\n";
      std::string regLeft8BitStr = registerTo8BitEquivalent(regLeft);
      output() << "setz " << regLeft8BitStr << "\n";
      output() << "movzx " << regLeft << ", " << regLeft8BitStr << "\n";
      swapLocation(regLeft, tempLeftLoc, node->location);
      removeLocation(regRight, node->right->location);
      break;
    }
    case ExpressionOperatorType::NOT_EQUALS: {
      output() << "cmp " << regLeft << ", " << regRight << "\n";
      std::string regLeft8BitStr = registerTo8BitEquivalent(regLeft);
      output() << "setnz " << regLeft8BitStr << "\n";
      output() << "movzx " << regLeft << ", " << regLeft8BitStr << "\n";
      swapLocation(regLeft, tempLeftLoc, node->location);
      removeLocation(regRight, node->right->location);
      break;
    }
    case ExpressionOperatorType::LESS_THAN: {
      output() << "cmp " << regLeft << ", " << regRight << "\n";
      std::string regLeft8BitStr = registerTo8BitEquivalent(regLeft);
      output() << "setl " << regLeft8BitStr << "\n";
      output() << "movzx " << regLeft << ", " << regLeft8BitStr << "\n";
      swapLocation(regLeft, tempLeftLoc, node->location);
      removeLocation(regRight, node->right->location);
      break;
    }
    case ExpressionOperatorType::LESS_THAN_OR_EQUAL: {
      output() << "cmp " << regLeft << ", " << regRight << "\n";
      std::string regLeft8BitStr = registerTo8BitEquivalent(regLeft);
      output() << "setle " << regLeft8BitStr << "\n";
      output() << "movzx " << regLeft << ", " << regLeft8BitStr << "\n";
      swapLocation(regLeft, tempLeftLoc, node->location);
      removeLocation(regRight, node->right->location);
      break;
    }
    case ExpressionOperatorType::GREATER_THAN: {
      output() << "cmp " << regLeft << ", " << regRight << "\n";
      std::string regLeft8BitStr = registerTo8BitEquivalent(regLeft);
      output() << "setg " << regLeft8BitStr << "\n";
      output() << "movzx " << regLeft << ", " << regLeft8BitStr << "\n";
      swapLocation(regLeft, tempLeftLoc, node->location);
      removeLocation(regRight, node->right->location);
      break;
    }
    case ExpressionOperatorType::GREATER_THAN_OR_EQUAL: {
      output() << "cmp " << regLeft << ", " << regRight << "\n";
      std::string regLeft8BitStr = registerTo8BitEquivalent(regLeft);
      output() << "setge " << regLeft8BitStr << "\n";
      output() << "movzx " << regLeft << ", " << regLeft8BitStr << "\n";
      swapLocation(regLeft, tempLeftLoc, node->location);
      removeLocation(regRight, node->right->location);
      break;
    }
    case ExpressionOperatorType::LOGICAL_AND: {
      std::cout << "LOGICAL_AND not implemented" << std::endl;
      file->fileStream->close();
      throw std::exception();
    }
    case ExpressionOperatorType::UNINITIALISED: {
      std::stringstream ssError;
      ssError << "BinOp UNINITIALISED";
      std::cout << ssError.str() << std::endl;
      ready({
                .mode = Data::Mode::ERROR,
                .type = Data::Type::MODE_CHANGE,
                .string = ssError.str(),
            });
      file->fileStream->close();
      throw std::exception();
    }
    default: {
      std::stringstream ssError;
      ssError << "BinOp not implemented: " << node->op;
      std::cout << ssError.str() << std::endl;
      ready({
                .mode = Data::Mode::ERROR,
                .type = Data::Type::MODE_CHANGE,
                .string = ssError.str(),
            });
      file->fileStream->close();
      throw std::exception();
    }
  }

  removeLocation(node->left->location, false);

  exitNode(node, "BinOp");

  return node->location;
}

Location* Generator::walkUnaryOp(ASTUnaryOp* node) {
  enterNode(node, "UnaryOp");

  walkExpression(node->child);
  auto* tempChildLoc = new Location();
  Register regChild = getRegisterForCopy(node->child->location, tempChildLoc);

  switch (node->op) {
    case ExpressionOperatorType::ADD:
      swapLocation(regChild, tempChildLoc, node->location);
      break;
    case ExpressionOperatorType::MINUS:
      output() << "xor " << Register::R15 << ", " << Register::R15 << ";zero\n";
      output() << "sub " << Register::R15 << ", " << regChild << "\n";
      output() << "mov " << regChild << ", " << Register::R15 << "\n";
      swapLocation(regChild, tempChildLoc, node->location);
      break;
    case ExpressionOperatorType::LOGICAL_NOT: {
      output() << "cmp " << regChild << ", " << 0 << "\n";
      std::string regChild8BitStr = registerTo8BitEquivalent(regChild);
      output() << "sete " << regChild8BitStr << "\n";
      output() << "movzx " << regChild << ", " << regChild8BitStr << "\n";
      swapLocation(regChild, tempChildLoc, node->location);
      break;
    }
    case ExpressionOperatorType::UNINITIALISED: {
      std::stringstream ssError;
      ssError << "UnaryOp UNINITIALISED" << node->op;
      std::cout << ssError.str() << std::endl;
      ready({
                .mode = Data::Mode::ERROR,
                .type = Data::Type::MODE_CHANGE,
                .string = ssError.str(),
            });
      file->fileStream->close();
      throw std::exception();
    }
    default: {
      std::stringstream ssError;
      ssError << "UnaryOp not implemented: " << node->op;
      std::cout << ssError.str() << std::endl;
      ready({
                .mode = Data::Mode::ERROR,
                .type = Data::Type::MODE_CHANGE,
                .string = ssError.str(),
            });
      file->fileStream->close();
      throw std::exception();
    }
  }

  removeLocation(node->child->location, false);

  exitNode(node, "UnaryOp");

  return node->location;
}

Location* Generator::walkLiteral(ASTLiteral* node) {
  enterNode(node, "Literal");

  switch (node->valueType) {
    case ASTLiteral::ValueType::STRING: {
      std::string str = std::string(node->value.stringData.data, node->value.stringData.count);
      std::string id = "ds" + std::to_string(constantIndex++);
      auto ret = constants.insert({str, id});
      if (ret.second == false) {
        id = ret.first->second;
      }
      ready({
                .mode = Data::Mode::CODE_GEN,
                .type = Data::Type::SPECIFIC,
                .codeGenState = Data::CodeGenState::PUSH_CONSTANT,
                .id = id,
                .string = str,
                .isNew = ret.second
            });
      getRegisterForConst(node->location, id);
//      output() << "mov rdi, " << id << "\n";
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

  exitNode(node, "Literal");

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

  exitNode(node, "VariableIdent");

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
      output() << "push " << reg;
      if (genComments) {
        output() << "; with " << loc;
      }
      output() << "\n";
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

    output() << "pop " << reg;
    if (genComments) {
      output() << "; with " << loc;
    }
    output() << "\n";


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
    output() << "mov " << reg << ", [rbp]\n";
    for (unsigned int i = 0; i < stackHeightDist - 1; ++i) {
      output() << "mov " << reg << ", [" << reg << "]\n";
    }
  } else {
    output() << "mov " << reg << ", rbp\n";
  }

  if (variable.parameter) {
    // Skips over procedure's return address and stackbase pointer on the stack
    constexpr unsigned int parameterOffset = 8;

    output() << "add " << reg << ", " << (parameterOffset + offset) << "\n";
  } else {
    offset += 1; // + 1 because we're using rbp instead of rsp
    output() << "sub " << reg << ", " << offset << "\n";
  }

  return reg;
}

void Generator::moveToMem(const std::string& ident, Location* location, unsigned int bytes) {
  BlockScope::Variable var = blockScopeStack.top().searchForVariable(ident);

  Register locRegister = locationMap.at(location);
  std::string locRegisterStr = registerToByteEquivalent(locRegister, bytes);
  if (genComments) {
    output() << ";mov " << *location << " to mem(ident: " << ident << ")\n";
  }
  Register varRegister = registerWithAddressForVariable(var);
  output() << "mov [" << varRegister << "], " << locRegisterStr << "\n";
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
    output() << "xor " << locRegister << ", " << locRegister << "\n";
  }
  output() << "mov " << locRegisterStr << ", [" << varRegister << "]\n";

  registerContents[locRegister] = var.location;
  locationMap.insert_or_assign(var.location, locRegister);
  ready({
            .mode = Data::Mode::CODE_GEN,
            .type = Data::Type::SPECIFIC,
            .codeGenState = Data::CodeGenState::SET_REG_AND_LOC,
            .reg = locRegister,
            .loc = var.location,
        });

  return var.location;
}

void Generator::recallFromParamRegister(Location* location) {
  Register newRegister = getAvailableRegister();

  Register exitingRegister = locationMap.at(location);

  ready({
            .mode = Data::Mode::CODE_GEN,
            .type = Data::Type::SPECIFIC,
            .codeGenState = Data::CodeGenState::MOVE_REG,
            .reg = newRegister,
            .reg2 = exitingRegister,
        });

  if (genComments) {
    output() << ";mov " << location << " (in " << exitingRegister << ") to " << newRegister << "\n";
  }
  output() << "mov " << newRegister << ", " << exitingRegister << "\n";

  registerContents[newRegister] = location;
  locationMap.insert_or_assign(location, newRegister);
  ready({
            .mode = Data::Mode::CODE_GEN,
            .type = Data::Type::SPECIFIC,
            .codeGenState = Data::CodeGenState::SET_REG_AND_LOC,
            .reg = newRegister,
            .loc = location,
        });
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

  Register oldRegister = locationMap.at(location);

  ready({
            .mode = Data::Mode::CODE_GEN,
            .type = Data::Type::SPECIFIC,
            .codeGenState = Data::CodeGenState::MOVE_REG,
            .reg = oldRegister,
            .reg2 = newRegister,
        });

  if (registerContents[newRegister] != nullptr) {
    std::cout << "WARNING: Register " << newRegister
              << " already has contents (" << registerContents[newRegister]
              << ") when asked to make way for " << *location
              << " (Relocating)" << std::endl;

    Register freeRegister = getAvailableRegister(excludingRegisters);
    Location* existingLoc = registerContents[newRegister];

    ready({
              .mode = Data::Mode::CODE_GEN,
              .type = Data::Type::SPECIFIC,
              .codeGenState = Data::CodeGenState::MOVE_REG,
              .reg = newRegister,
              .reg2 = freeRegister,
              .keep = true,
          });

    if (genComments) {
      output() << ";mov " << *existingLoc << " to " << freeRegister << " (relocation)" << "\n";
    }
    output() << "mov " << freeRegister << ", " << newRegister << "\n";

    registerContents[freeRegister] = existingLoc;
    locationMap.insert_or_assign(existingLoc, freeRegister);
    ready({
              .mode = Data::Mode::CODE_GEN,
              .type = Data::Type::SPECIFIC,
              .codeGenState = Data::CodeGenState::SET_REG_AND_LOC,
              .reg = freeRegister,
              .loc = existingLoc,
          });
  }

  if (genComments) {
    output() << ";mov " << *location << " to " << newRegister << "\n";
  }
  output() << "mov " << newRegister << ", " << oldRegister << "\n";

  registerContents[oldRegister] = nullptr;
  ready({
            .mode = Data::Mode::CODE_GEN,
            .type = Data::Type::SPECIFIC,
            .codeGenState = Data::CodeGenState::SET_REG,
            .reg = oldRegister,
        });
  registerContents[newRegister] = location;
  locationMap.insert_or_assign(location, newRegister);
  ready({
            .mode = Data::Mode::CODE_GEN,
            .type = Data::Type::SPECIFIC,
            .codeGenState = Data::CodeGenState::SET_REG_AND_LOC,
            .reg = newRegister,
            .loc = location,
        });
}

void Generator::swapLocation(Register reg, Location* oldLoc, Location* newLoc, bool forceRmParam) {
  registerContents[reg] = newLoc;
  ready({
            .mode = Data::Mode::CODE_GEN,
            .type = Data::Type::SPECIFIC,
            .codeGenState = Data::CodeGenState::SET_REG,
            .reg = reg,
            .loc = newLoc,
        });

  if (oldLoc && (!oldLoc->isParameter || forceRmParam)) {
    locationMap.erase(oldLoc);
    ready({
              .mode = Data::Mode::CODE_GEN,
              .type = Data::Type::SPECIFIC,
              .codeGenState = Data::CodeGenState::SET_LOC,
              .loc = oldLoc,
          });
  }
  locationMap.insert_or_assign(newLoc, reg);
  ready({
            .mode = Data::Mode::CODE_GEN,
            .type = Data::Type::SPECIFIC,
            .codeGenState = Data::CodeGenState::SET_LOC,
            .loc = newLoc,
            .reg = reg,
        });
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
  ready({
            .mode = Data::Mode::CODE_GEN,
            .type = Data::Type::SPECIFIC,
            .codeGenState = Data::CodeGenState::SET_REG,
            .reg = reg,
        });

  if (oldLoc) {
    locationMap.erase(oldLoc);
    ready({
              .mode = Data::Mode::CODE_GEN,
              .type = Data::Type::SPECIFIC,
              .codeGenState = Data::CodeGenState::SET_LOC,
              .loc = oldLoc,
          });
  } else {
    locationMap.erase(existingLoc);
    ready({
              .mode = Data::Mode::CODE_GEN,
              .type = Data::Type::SPECIFIC,
              .codeGenState = Data::CodeGenState::SET_LOC,
              .loc = existingLoc,
          });
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
  ready({
            .mode = Data::Mode::CODE_GEN,
            .type = Data::Type::SPECIFIC,
            .codeGenState = Data::CodeGenState::SET_REG,
            .reg = reg,
        });
  locationMap.erase(oldLoc);
  ready({
            .mode = Data::Mode::CODE_GEN,
            .type = Data::Type::SPECIFIC,
            .codeGenState = Data::CodeGenState::SET_LOC,
            .loc = oldLoc,
        });
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
    std::stringstream ssError;
    ssError << "No available register!";
    std::cout << ssError.str() << std::endl;
    ready({
              .mode = Data::Mode::ERROR,
              .type = Data::Type::MODE_CHANGE,
              .string = ssError.str(),
          });
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
    std::stringstream ssError;
    ssError << "No available register!";
    std::cout << ssError.str() << std::endl;
    ready({
              .mode = Data::Mode::ERROR,
              .type = Data::Type::MODE_CHANGE,
              .string = ssError.str(),
          });
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
      ready({
                .mode = Data::Mode::CODE_GEN,
                .type = Data::Type::SPECIFIC,
                .codeGenState = Data::CodeGenState::MOVE_CONST,
                .reg = availableRegister,
            });

      output() << "mov " << availableRegister << ", " << constant << "\n";
      registerContents[availableRegister] = location;
      locationMap.insert_or_assign(location, availableRegister);
      ready({
                .mode = Data::Mode::CODE_GEN,
                .type = Data::Type::SPECIFIC,
                .codeGenState = Data::CodeGenState::SET_REG_AND_LOC,
                .reg = availableRegister,
                .loc = location,
            });
    } else {
      moveToRegister(availableRegister, location);
    }
    return availableRegister;
  }

  // Else, error (for now; TODO)
  std::stringstream ssError;
  ssError << "No register for " << *location;
  std::cout << ssError.str() << std::endl;
  ready({
            .mode = Data::Mode::ERROR,
            .type = Data::Type::MODE_CHANGE,
            .string = ssError.str(),
        });
  file->fileStream->close();
  throw std::exception();
}

Register Generator::getRegisterForCopy(Location* location, Location* newLocation) {
  Register newRegister = getAvailableRegister();

#ifndef NDEBUG
  dumpRegisters(true);
#endif

  Register oldRegister = locationMap.at(location);

  ready({
            .mode = Data::Mode::CODE_GEN,
            .type = Data::Type::SPECIFIC,
            .codeGenState = Data::CodeGenState::MOVE_REG,
            .reg = newRegister,
            .reg2 = oldRegister,
        });

  if (genComments) {
    output() << ";mov " << *location << " to " << newRegister << "\n";
  }
  output() << "mov " << newRegister << ", " << oldRegister << "\n";

  registerContents[newRegister] = location;
  ready({
            .mode = Data::Mode::CODE_GEN,
            .type = Data::Type::SPECIFIC,
            .codeGenState = Data::CodeGenState::SET_REG,
            .reg = newRegister,
            .loc = location,
        });
  locationMap.insert_or_assign(newLocation, newRegister);
  ready({
            .mode = Data::Mode::CODE_GEN,
            .type = Data::Type::SPECIFIC,
            .codeGenState = Data::CodeGenState::SET_LOC,
            .reg = newRegister,
            .loc = newLocation,
        });

  return newRegister;
}

void Generator::comment(const std::string& comment) {
  if (!genComments) return;

  output() << ";" << comment << "\n";
}

void Generator::dumpRegisters(bool mismatchCheckOnly) {
  int regCount = 0;
  int locCount = 0;

  // Count registers
  for (int i = 0; i < TOTAL_REGISTERS; ++i) {
    auto reg = (Register) i;
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
    output() << ";Dump registers:\n";
  }
  std::cout << "Dump registers:" << std::endl;
  for (int i = 0; i < TOTAL_REGISTERS; ++i) {
    auto reg = (Register) i;
    Location* loc = registerContents[reg];
    if (loc) {
      if (genComments) {
        output() << "; - " << reg << ": " << loc->id << "\n";
      }
      std::cout << "- " << reg << ": " << loc->id << std::endl;
    } else {
      if (genComments) {
        output() << "; - " << reg << ": -" << "\n";
      }
      std::cout << "- " << reg << ": -" << std::endl;
    }
  }

  // Print locations
  if (genComments) {
    output() << ";Dump locations:\n";
  }
  std::cout << "Dump locations:" << std::endl;
  for (auto pair : locationMap) {
    Location* loc = pair.first;
    Register reg = pair.second;
    if (reg != Register::NONE) {
      if (genComments) {
        output() << "; - " << loc->id << ": " << reg << "\n";
      }
      std::cout << "- " << loc->id << ": " << reg << std::endl;
    } else {
      if (genComments) {
        output() << "; - " << loc->id << ": -" << "\n";
      }
      std::cout << "- " << loc->id << ": -" << std::endl;
    }
  }

  // Print mismatch
  if (locCount != regCount) {
    if (genComments) {
      output() << "; !!!MISMATCH!!!\n";
      output() << "; !!!MISMATCH!!!\n";
    }
    std::cout << "!!!MISMATCH!!!" << std::endl;
  }
}

Generator::StreamHelper& Generator::output() {
  flushOutput();
  _isOutputFlushed = false;
  return _output;
}

void Generator::flushOutput() {
  if (!_isOutputFlushed) {
    ready({
              .mode = Data::Mode::CODE_GEN,
              .type = Data::Type::SPECIFIC,
              .codeGenState = Data::CodeGenState::OUTPUT,
              .string = _output.ss.str(),
          });
    _output.ss.str(std::string());
    _isOutputFlushed = true;
  }
}

void Generator::enterNode(ASTNode* node, const std::string& commentName) {
  ready({
            .mode = Data::Mode::CODE_GEN,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .codeGenState = Data::CodeGenState::ENTER_NODE,
            .nodeId = node->nodeId,
        });
  comment("BEGIN " + commentName);
}

void Generator::exitNode(ASTNode* node, const std::string& commentName) {
  comment("END " + commentName);
  ready({
            .mode = Data::Mode::CODE_GEN,
            .type = Data::Type::SPECIFIC,
            .nodeType = node->type,
            .codeGenState = Data::CodeGenState::EXIT_NODE,
            .nodeId = node->nodeId,
        });
}

#pragma clang diagnostic pop
