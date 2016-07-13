//
// Created by Markus on 13.07.2016.
//

#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include "toplevel.h"
#include "optimizer.h"
#include "jit.h"

static void HandleDefinition() {
    if (auto FnAST = ParseDefinition()) {
        if (auto *FnIR = FnAST->codegen()) {
            fprintf(stderr, "Read function definition:");
            FnIR->dump();
            TheJIT->addModule(std::move(TheModule));
            InitializeModuleAndPassManager();
        }
    } else {
        // Skip token for error recovery.
        getNextToken();
    }
}

static void HandleExtern() {
    if (auto ProtoAST = ParseExtern()) {
        if (auto *FnIR = ProtoAST->codegen()) {
            fprintf(stderr, "Read extern: ");
            FnIR->dump();
            FunctionProtos[ProtoAST->getName()] = std::move(ProtoAST);
        }
    } else {
        // Skip token for error recovery.
        getNextToken();
    }
}

static void HandleTopLevelExpression() {
    // Evaluate a top-level expression into an anonymous function.
    if (auto FnAST = ParseTopLevelExpr()) {
        if (FnAST->codegen()) {

            // JIT the module containing the anonymous expression, keeping a handle so
            // we can free it later.
            auto H = TheJIT->addModule(move(TheModule));
            InitializeModuleAndPassManager();

            // Search the JIT for the __anon_expr symbol.
            auto ExprSymbol = TheJIT->findSymbol("__anon_expr");
            assert(ExprSymbol && "Function not found");

            // Get the symbol's address and cast it to the right type (takes no
            // arguments, returns a double) so we can call it as a native function.
            double (*FP)() = (double (*)())(intptr_t)ExprSymbol.getAddress();
            fprintf(stderr, "Evaluated to %f\n", FP());

            // Delete the anonymous expression module from the JIT.
            TheJIT->removeModule(H);

        }
    } else {
        // Skip token for error recovery.
        getNextToken();
    }
}

//! top ::= definition | external | expression | ';'
void MainLoop() {
    while (1) {
        fprintf(stderr, "ready> ");
        switch (CurTok) {
            case static_cast<int>(Token::EndOfFile): {
                return;
            }
            case ';': { // ignore top-level semicolons.
                getNextToken();
                break;
            }
            case static_cast<int>(Token::FunctionDefinition): {
                HandleDefinition();
                break;
            }
            case static_cast<int>(Token::ExternKeyword): {
                HandleExtern();
                break;
            }
            default: {
                HandleTopLevelExpression();
                break;
            }
        }
    }
}