//
// Created by Markus on 14.07.2016.
//

#include <llvm/IR/PassManager.h>
#include <llvm/IR/LegacyPassManager.h>
#include "codegen.h"
#include "optimizer.h"
#include "jit.h"
#include "helper.h"

#include <llvm/Analysis/BasicAliasAnalysis.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>

using namespace helper;

unique_ptr<legacy::FunctionPassManager> TheFPM;

void InitializeModuleAndPassManager(void) {
    // Open a new module.
    TheModule = helper::make_unique<Module>("Kaleidoscope Tutorial JIT", TheContext);
    TheModule->setDataLayout(TheJIT->getTargetMachine().createDataLayout());

    // Create a new pass manager attached to it.
    TheFPM = helper::make_unique<legacy::FunctionPassManager>(TheModule.get());

    // Do simple "peephole" optimizations and bit-twiddling optzns.
    TheFPM->add(createInstructionCombiningPass());

    // Reassociate expressions.
    TheFPM->add(createReassociatePass());

    // Eliminate Common SubExpressions.
    TheFPM->add(createGVNPass());

    // Simplify the control flow graph (deleting unreachable blocks, etc).
    TheFPM->add(createCFGSimplificationPass());

    TheFPM->doInitialization();
}