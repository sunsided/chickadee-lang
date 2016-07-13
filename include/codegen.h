//
// Created by Markus on 13.07.2016.
//

#ifndef CHICKADEE_CODEGEN_H
#define CHICKADEE_CODEGEN_H

#include <memory>
#include <string>
#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>

using namespace std;
using namespace llvm;

//! TheContext is an opaque object that owns a lot of core LLVM data structures,
//! such as the type and constant value tables.
extern LLVMContext TheContext;

//! TheModule is an LLVM construct that contains functions and global variables. In many ways, it is the top-level
//! structure that the LLVM IR uses to contain code.
//! It will own the memory for all of the IR that we generate, which is why the codegen() method returns
//! a raw Value*, rather than a unique_ptr<Value>.
extern unique_ptr<Module> TheModule;

Value *LogErrorV(const char *Str);

#endif //CHICKADEE_CODEGEN_H
