#include <string>
#include <memory>
#include <map>

#include "lexer.h"
#include "parser.h"
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

    // Run the main "interpreter loop" now.
    MainLoop();

    return 0;
}