//
// Created by Markus on 13.07.2016.
//

#include "lexer.h"
#include "parser.h"
#include "toplevel.h"

static void HandleDefinition() {
    if (ParseDefinition()) {
        fprintf(stderr, "Parsed a function definition.\n");
    } else {
        // Skip token for error recovery.
        getNextToken();
    }
}

static void HandleExtern() {
    if (ParseExtern()) {
        fprintf(stderr, "Parsed an extern\n");
    } else {
        // Skip token for error recovery.
        getNextToken();
    }
}

static void HandleTopLevelExpression() {
    // Evaluate a top-level expression into an anonymous function.
    if (ParseTopLevelExpr()) {
        fprintf(stderr, "Parsed a top-level expr\n");
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