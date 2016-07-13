//
// Created by Markus on 13.07.2016.
//

#include <memory>
#include <map>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>

#include "codegen.h"
#include "parser.h"

using namespace std;
using namespace llvm;

//! TheContext is an opaque object that owns a lot of core LLVM data structures,
//! such as the type and constant value tables.
LLVMContext TheContext;

//! The Builder object is a helper object that makes it easy to generate LLVM instructions.
//! Instances of the IRBuilder class template keep track of the current place to insert instructions and has
//! methods to create new instructions.
static IRBuilder<> Builder(TheContext);

//! TheModule is an LLVM construct that contains functions and global variables. In many ways, it is the top-level
//! structure that the LLVM IR uses to contain code.
//! It will own the memory for all of the IR that we generate, which is why the codegen() method returns
//! a raw Value*, rather than a unique_ptr<Value>.
unique_ptr<Module> TheModule;

//! The NamedValues map keeps track of which values are defined in the current scope and what their
//! LLVM representation is. (In other words, it is a symbol table for the code).
static map<string, Value *> NamedValues;

Value *LogErrorV(const char *Str) {
    LogError(Str);
    return nullptr;
}

Value *NumberExprAST::codegen() {
    return ConstantFP::get(TheContext, APFloat(_val));
}

Value *VariableExprAST::codegen() {
    // Look this variable up in the function.
    Value *V = NamedValues[_name];
    if (!V) {
        LogErrorV("Unknown variable name");
    }
    return V;
}

Value *BinaryExprAST::codegen() {
    Value *L = LHS->codegen();
    Value *R = RHS->codegen();
    if (!L || !R) {
        return nullptr;
    }

    switch (_op) {
        case '+': {
            return Builder.CreateFAdd(L, R, "addtmp");
        }
        case '-': {
            return Builder.CreateFSub(L, R, "subtmp");
        }
        case '*': {
            return Builder.CreateFMul(L, R, "multmp");
        }
        case '<': {
            L = Builder.CreateFCmpULT(L, R, "cmptmp");
            // Convert bool 0/1 to double 0.0 or 1.0
            return Builder.CreateUIToFP(L, Type::getDoubleTy(TheContext), "booltmp");
        }
        default: {
            return LogErrorV("invalid binary operator");
        }
    }
}

Value *CallExprAST::codegen() {
    // Look up the name in the global module table.
    Function *CalleeF = TheModule->getFunction(_callee);
    if (!CalleeF) {
        return LogErrorV("Unknown function referenced");
    }

    // If argument mismatch error.
    if (CalleeF->arg_size() != _args.size()) {
        return LogErrorV("Incorrect # arguments passed");
    }

    std::vector<Value *> ArgsV;
    for (unsigned i = 0, e = _args.size(); i != e; ++i) {
        ArgsV.push_back(_args[i]->codegen());
        if (!ArgsV.back()) {
            return nullptr;
        }
    }

    return Builder.CreateCall(CalleeF, ArgsV, "calltmp");
}

Function *PrototypeAST::codegen() {
    // Make the function type:  double(double,double) etc.
    std::vector<Type *> Doubles(_args.size(), Type::getDoubleTy(TheContext));
    FunctionType *FT = FunctionType::get(Type::getDoubleTy(TheContext), Doubles, false);

    Function *F = Function::Create(FT, Function::ExternalLinkage, _name, TheModule.get());

    // Set names for all arguments.
    unsigned Idx = 0;
    for (auto &Arg : F->args()) {
        Arg.setName(_args[Idx++]);
    }

    return F;
}

Function *FunctionAST::codegen() {
    // First, check for an existing function from a previous 'extern' declaration.
    Function *TheFunction = TheModule->getFunction(_proto->getName());

    if (!TheFunction) {
        TheFunction = _proto->codegen();
    }

    if (!TheFunction) {
        return nullptr;
    }

    if (!TheFunction->empty()) {
        return (Function *) LogErrorV("Function cannot be redefined.");
    }

    // Create a new basic block to start insertion into.
    BasicBlock *BB = BasicBlock::Create(TheContext, "entry", TheFunction);
    Builder.SetInsertPoint(BB);

    // Record the function arguments in the NamedValues map.
    NamedValues.clear();
    for (auto &Arg : TheFunction->args()) {
        NamedValues[Arg.getName()] = &Arg;
    }

    if (Value *RetVal = _body->codegen()) {
        // Finish off the function.
        Builder.CreateRet(RetVal);

        // Validate the generated code, checking for consistency.
        // defined in llvm/IR/Verifier.h
        verifyFunction(*TheFunction);

        return TheFunction;
    }

    // Error reading body, remove function.
    TheFunction->eraseFromParent();
    return nullptr;
}