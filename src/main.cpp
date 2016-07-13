#include <string>
#include <memory>
#include <map>
#include <optimizer.h>

#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include "jit.h"
#include "toplevel.h"

//! printd - printf that takes a double prints it as "%f\n", returning 0.
//! intended to be used as "extern printd(x);"
extern "C" double printd(double X) {
    fprintf(stderr, "%f\n", X);
    return 0;
}

int main() {
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();
    LLVMInitializeNativeAsmParser();

    // Install standard binary operators.
    // 1 is lowest precedence.
    BinOpPrecedence['<'] = 10;
    BinOpPrecedence['+'] = 20;
    BinOpPrecedence['-'] = 20;
    BinOpPrecedence['*'] = 40;  // highest.

    // Prime the first token.
    fprintf(stderr, "ready> ");
    getNextToken();

    // prepare the Just-in-Time compiler
    TheJIT = make_unique<KaleidoscopeJIT>();
    InitializeModuleAndPassManager();

    // Run the main "interpreter loop" now.
    MainLoop();

    return 0;
}