//
// Created by Markus on 14.07.2016.
//

#ifndef CHICKADEE_JIT_H
#define CHICKADEE_JIT_H

#include <memory>
#include <llvm/IR/LegacyPassManager.h>
#include "KaleidoscopeJIT.h"
#include "ast.h"

using namespace std;
using namespace llvm;
using namespace llvm::orc;

extern unique_ptr<KaleidoscopeJIT> TheJIT;
extern unique_ptr<legacy::FunctionPassManager> TheFPM;
extern map<string, unique_ptr<PrototypeAST>> FunctionProtos;

#endif //CHICKADEE_JIT_H
