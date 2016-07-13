#include <string>
#include <memory>
#include <map>

#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include "toplevel.h"

int main() {
    // Install standard binary operators.
    // 1 is lowest precedence.
    BinOpPrecedence['<'] = 10;
    BinOpPrecedence['+'] = 20;
    BinOpPrecedence['-'] = 20;
    BinOpPrecedence['*'] = 40;  // highest.

    // Prime the first token.
    fprintf(stderr, "ready> ");
    getNextToken();

    // Make the module, which holds all the code.
    TheModule = llvm::make_unique<Module>("my cool jit", TheContext);

    // Run the main "interpreter loop" now.
    MainLoop();

    // Print out all of the generated code.
    TheModule->dump();

    return 0;
}