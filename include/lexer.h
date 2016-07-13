//
// Created by Markus on 13.07.2016.
//

#ifndef CHICKADEE_LEXER_H
#define CHICKADEE_LEXER_H

#include <string>

using namespace std;

// The lexer returns tokens [0-255] if it is an unknown character, otherwise one
// of these for known things.
enum class Token : int {
    EndOfFile = -1,

    // commands
    FunctionDefinition = -2,
    ExternKeyword = -3,

    // primary
    Identifier = -4,
    Number = -5,
};

extern string IdentifierStr; // Filled in if Identifier
extern double NumVal;        // Filled in if Number

int getToken();

//! CurTok/getNextToken - Provide a simple token buffer.  CurTok is the current
//! token the parser is looking at.  getNextToken reads another token from the
//! lexer and updates CurTok with its results.
extern int CurTok;
int getNextToken();

#endif //CHICKADEE_LEXER_H
