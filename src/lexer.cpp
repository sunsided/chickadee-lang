//
// Created by Markus on 13.07.2016.
//

#include "lexer.h"

string IdentifierStr; // Filled in if Identifier
double NumVal;        // Filled in if Number
int CurTok;

//! getToken - Return the next token from standard input.
int getToken() {
    static int lastChar = ' ';

    // Skip any whitespace.
    while (isspace(lastChar)) {
        lastChar = getchar();
    }

    if (isalpha(lastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
        IdentifierStr = lastChar;
        while (isalnum((lastChar = getchar()))) {
            IdentifierStr += lastChar;
        }

        if (IdentifierStr == "def") {
            return static_cast<int>(Token::FunctionDefinition);
        }
        if (IdentifierStr == "extern") {
            return static_cast<int>(Token::ExternKeyword);
        }
        return static_cast<int>(Token::Identifier);
    }

    if (isdigit(lastChar) || lastChar == '.') {   // Number: [0-9.]+
        std::string NumStr;
        do {
            NumStr += lastChar;
            lastChar = getchar();
        } while (isdigit(lastChar) || lastChar == '.');

        NumVal = stod(NumStr, 0);
        return static_cast<int>(Token::Number);
    }

    if (lastChar == '#') {
        // Comment until end of line.
        do {
            lastChar = getchar();
        }
        while (lastChar != EOF && lastChar != '\n' && lastChar != '\r');

        if (lastChar != EOF)
            return getToken();
    }

    // Check for end of file.  Don't eat the EOF.
    if (lastChar == EOF) {
        return static_cast<int>(Token::EndOfFile);
    }

    // Otherwise, just return the character as its ascii value.
    int thisChar = lastChar;
    lastChar = getchar();
    return thisChar;
}

int getNextToken() {
    return CurTok = getToken();
}