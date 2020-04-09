//
// Created by Callum Todd on 2020/03/23.
//

#include <iostream>
#include <sstream>
#include "Generator.h"
#include "AST.h"

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
  comment("BEGIN leading boilerplate");
  *file->fileStream <<
                    "global    main\n"
                    "extern    printf\n"
                    "extern    puts\n"
                    "\n"
                    "section   .text\n";
  comment("END leading boilerplate");

  if (astRoot->type == ASTType::BLOCK) {
    walkBlock(static_cast<ASTBlock*>(astRoot));
  } else {
    std::cout << "Root is not block. Bad." << std::endl;
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

#if 1
    std::string strSegment;
    while (charIndex < str.size() && str[charIndex] != '\\') {
      strSegment.push_back(str[charIndex]);
      charIndex++;
    }
    *file->fileStream << "\"" << strSegment << "\"";
#else
    *file->fileStream << (int)str[charIndex++];
#endif
  }
}

void Generator::walkBlock(ASTBlock* node) {
  comment("BEGIN Block");
  for (auto statement : node->statements) {
    walkStatement(statement);
  }
  comment("END Block");
}

void Generator::walkStatement(ASTStatement* node) {
  switch (node->type) {
    case UNINITIALISED:
      std::cout << "Statement uninitialised" << std::endl;
      throw std::exception();
    case BLOCK:
      walkStatement(static_cast<ASTBlock*>(node));
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
      throw std::exception();
  }
}

void Generator::walkVariableDeclaration(ASTVariableDeclaration* node) {

}

void Generator::walkProcedureDeclaration(ASTProcedure* node) {
  comment("BEGIN ProcedureDeclaration");

  *file->fileStream << node->ident << ":\n";
  walkBlock(node->block);
  *file->fileStream << "ret\n";

  comment("END ProcedureDeclaration");
}

void Generator::walkProcedureCall(ASTProcedureCall* node) {
  comment("BEGIN ProcedureCall");

  std::vector<Location*> paramLocs;
  for (auto param : node->parameters) {
    paramLocs.push_back(walkExpression(param));
//    walkExpression(param);
  }

  pushCallerSaved();

  // Print f, expects
//  *file->fileStream << "mov rdi, format\n"; //set 1st parameter (format)
//  *file->fileStream << "mov rsi, rax\n"; //set 2nd parameter (current_number)
  int possibleRegistersInCallBeforeStack = 2;
  Register registerOrder[] = {
      Register::RDI,
      Register::RSI,
  };
  for (size_t i = 0; i < paramLocs.size(); ++i) {
    if (i < possibleRegistersInCallBeforeStack) {
      moveToRegister(registerOrder[i], paramLocs[i]);
    } else {
      std::cout << "Not implemented yet" << std::endl;
      throw std::exception();
    }
  }
  *file->fileStream << "xor rax, rax\n"; //zero rax because printf is varargs

  *file->fileStream << "call " << node->ident << "\n";

  popCallerSaved();

  for (size_t i = 0; i < paramLocs.size(); ++i) {
    if (i < possibleRegistersInCallBeforeStack) {
      Location* loc = paramLocs[i];
      Register reg = locationMap.at(loc);
      registerContents[reg] = nullptr;
      locationMap.erase(loc);
    } else {
      std::cout << "Not implemented yet" << std::endl;
      throw std::exception();
    }
  }

  comment("END ProcedureCall");
}

void Generator::walkVariableAssignment(ASTVariableAssignment* node) {

}

void Generator::walkIf(ASTIf* node) {

}

void Generator::walkWhile(ASTWhile* node) {

}

void Generator::walkContinue(ASTContinue* node) {

}

void Generator::walkBreak(ASTBreak* node) {

}

void Generator::walkReturn(ASTReturn* node) {

}

Location* Generator::walkBinOp(ASTBinOp* node) {
  comment("BEGIN BinOp");

  walkExpression(node->left);
  walkExpression(node->right);
  Register regLeft = getRegisterFor(node->left->location);
  Register regRight = getRegisterFor(node->right->location);

  switch (node->op) {
    case ExpressionOperatorType::ADD:
      *file->fileStream << "add " << regLeft << ", " << regRight << "\n";
      registerContents[regLeft] = node->location;
      locationMap.insert_or_assign(node->location, regLeft);

      // Remove left & right locations
      registerContents[regRight] = nullptr;
      locationMap.erase(node->left->location);
      locationMap.erase(node->right->location);
      break;
    case ExpressionOperatorType::MINUS:
      *file->fileStream << "sub " << regLeft << ", " << regRight << "\n";
      registerContents[regLeft] = node->location;
      locationMap.insert_or_assign(node->location, regLeft);

      // Remove left & right locations
      registerContents[regRight] = nullptr;
      locationMap.erase(node->left->location);
      locationMap.erase(node->right->location);
      break;
    case ExpressionOperatorType::MULTIPLY: {
      *file->fileStream << "imul " << regLeft << ", " << regRight << "\n";
      registerContents[regLeft] = node->location;
      locationMap.insert_or_assign(node->location, regLeft);

      // Remove left & right locations
      registerContents[regRight] = nullptr;
      locationMap.erase(node->left->location);
      locationMap.erase(node->right->location);
      break;
    }
    case ExpressionOperatorType::DIVIDE: {
      *file->fileStream << "idiv " << regLeft << ", " << regRight << "\n";
      registerContents[regLeft] = node->location;
      locationMap.insert_or_assign(node->location, regLeft);

      // Remove left & right locations
      registerContents[regRight] = nullptr;
      locationMap.erase(node->left->location);
      locationMap.erase(node->right->location);
      break;
    }
    case ExpressionOperatorType::UNINITIALISED:
      std::cout << "BinOp UNINITIALISED" << std::endl;
      throw std::exception();
    default:
      std::cout << "BinOp not implemented: " << node->op << std::endl;
      throw std::exception();
  }

  comment("END BinOp");

  return node->location;
}

Location* Generator::walkUnaryOp(ASTUnaryOp* node) {
  std::cout << "UnaryOp didn't return a location" << std::endl;
  throw std::exception();
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
  std::cout << "VariableIdent didn't return a location" << std::endl;
  throw std::exception();
}

void Generator::pushCallerSaved() {
  comment("BEGIN pushCallerSaved");



  comment("END pushCallerSaved");
}

void Generator::popCallerSaved() {
  comment("BEGIN popCallerSaved");



  comment("END popCallerSaved");
}

void Generator::moveToRegister(Register reg, Location* location) {
  if (registerContents[reg] == location) {
    return;
  }

  if (registerContents[reg] != nullptr) {
    std::cout << "WARNING: Register " << reg
              << " already has contents (" << registerContents[reg]
              << ") when asked to make way for: " << *location << std::endl;

    Location* loc = registerContents[reg];
    locationMap.insert_or_assign(loc, Register::NONE);
  }


  Register locRegister = locationMap.at(location);
  if (genComments) {
    *file->fileStream << ";mov " << *location << " to " << reg << "\n";
  }
  *file->fileStream << "mov " << reg << ", " << locRegister << "\n";

  registerContents[reg] = location;
  locationMap.insert_or_assign(location, reg);
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
  throw std::exception();
}

void Generator::comment(const std::string& comment) {
  if (!genComments) return;

  *file->fileStream << ";" << comment << "\n";
}
