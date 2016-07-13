//
// Created by Markus on 14.07.2016.
//

#include "KaleidoscopeJIT.h"
#include "jit.h"
#include "ast.h"

using namespace llvm::orc;

unique_ptr<KaleidoscopeJIT> TheJIT;
map<string, unique_ptr<PrototypeAST>> FunctionProtos;