#include <string>
#include <iostream>
#include <memory>
#include <vector>
#include <map>

using namespace std;

namespace helper {
    // Cloning make_unique here until it's standard in C++14.
    // Using a namespace to avoid conflicting with MSVC's std::make_unique (which
    // ADL can sometimes find in unqualified calls).
    template <class T, class... Args>
    static
    typename std::enable_if<!std::is_array<T>::value, std::unique_ptr<T>>::type
    make_unique(Args &&... args) {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
} // end namespace helper

using namespace helper;

//===----------------------------------------------------------------------===//
// Lexer
//===----------------------------------------------------------------------===//

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

static std::string IdentifierStr; // Filled in if Identifier
static double NumVal;             // Filled in if Number

//! getToken - Return the next token from standard input.
static int getToken() {
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

//===----------------------------------------------------------------------===//
// Abstract Syntax Tree (aka Parse Tree)
//===----------------------------------------------------------------------===//

//! ExprAST - Base class for all expression nodes.
class ExprAST {
public:
    virtual ~ExprAST() {}
};

//! NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST {
    double Val;

public:
    NumberExprAST(double Val) : Val(Val) {}
};

//! VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST {
    string Name;

public:
    VariableExprAST(const string &Name) : Name(Name) {}
};

//! BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST {
    char Op;
    unique_ptr<ExprAST> LHS, RHS;

public:
    BinaryExprAST(char op, unique_ptr<ExprAST> LHS, unique_ptr<ExprAST> RHS)
            : Op(op), LHS(move(LHS)), RHS(move(RHS)) {}
};

//! CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
    string Callee;
    vector<unique_ptr<ExprAST>> Args;

public:
    CallExprAST(const string &Callee, vector<unique_ptr<ExprAST>> Args)
            : Callee(Callee), Args(std::move(Args)) {}
};

//! PrototypeAST - This class represents the "prototype" for a function,
//! which captures its name, and its argument names (thus implicitly the number
//! of arguments the function takes).
class PrototypeAST {
    string Name;
    vector<string> Args;

public:
    PrototypeAST(const string &name, vector<string> Args)
            : Name(name), Args(move(Args)) {}
};

//! FunctionAST - This class represents a function definition itself.
class FunctionAST {
    unique_ptr<PrototypeAST> Proto;
    unique_ptr<ExprAST> Body;

public:
    FunctionAST(unique_ptr<PrototypeAST> Proto, unique_ptr<ExprAST> Body)
            : Proto(move(Proto)), Body(move(Body)) {}
};

//===----------------------------------------------------------------------===//
// Parser
//===----------------------------------------------------------------------===//

static unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec, unique_ptr<ExprAST> LHS);
static std::unique_ptr<ExprAST> ParseExpression();

//! CurTok/getNextToken - Provide a simple token buffer.  CurTok is the current
//! token the parser is looking at.  getNextToken reads another token from the
//! lexer and updates CurTok with its results.
static int CurTok;
static int getNextToken() {
    return CurTok = getToken();
}

//! LogError* - These are little helper functions for error handling.
unique_ptr<ExprAST> LogError(const char *Str) {
    fprintf(stderr, "LogError: %s\n", Str);
    return nullptr;
}

unique_ptr<PrototypeAST> LogErrorP(const char *Str) {
    LogError(Str);
    return nullptr;
}

//! numberexpr ::= number
static unique_ptr<ExprAST> ParseNumberExpr() {
    auto Result = make_unique<NumberExprAST>(NumVal);
    getNextToken(); // consume the number
    return Result;
}

//! parenexpr ::= '(' expression ')'
static unique_ptr<ExprAST> ParseParenExpr() {
    getNextToken(); // eat (.
    auto V = ParseExpression();
    if (!V)
        return nullptr;

    if (CurTok != ')') {
        return LogError("expected ')'");
    }
    getNextToken(); // eat ).
    return V;
}

//! identifierexpr
//!   ::= identifier
//!   ::= identifier '(' expression* ')'
static unique_ptr<ExprAST> ParseIdentifierExpr() {
    string IdName = IdentifierStr;

    getNextToken();  // eat identifier.

    if (CurTok != '(') // Simple variable ref.
        return make_unique<VariableExprAST>(IdName);

    // Call.
    getNextToken();  // eat (
    vector<unique_ptr<ExprAST>> Args;
    if (CurTok != ')') {
        while (1) {
            if (auto Arg = ParseExpression())
                Args.push_back(std::move(Arg));
            else
                return nullptr;

            if (CurTok == ')')
                break;

            if (CurTok != ',')
                return LogError("Expected ')' or ',' in argument list");
            getNextToken();
        }
    }

    // Eat the ')'.
    getNextToken();

    return make_unique<CallExprAST>(IdName, std::move(Args));
}

//! primary
//!   ::= identifierexpr
//!   ::= numberexpr
//!   ::= parenexpr
static unique_ptr<ExprAST> ParsePrimary() {
    switch (CurTok) {
        default:
            return LogError("unknown token when expecting an expression");
        case static_cast<int>(Token::Identifier):
            return ParseIdentifierExpr();
        case static_cast<int>(Token::Number):
            return ParseNumberExpr();
        case '(':
            return ParseParenExpr();
    }
}

/// BinopPrecedence - This holds the precedence for each binary operator that is
/// defined.
static map<char, int> BinopPrecedence;

//! GetTokPrecedence - Get the precedence of the pending binary operator token.
static int GetTokPrecedence() {
    if (!isascii(CurTok)) {
        return -1;
    }

    // Make sure it's a declared binop.
    int TokPrec = BinopPrecedence[CurTok];
    if (TokPrec <= 0) {
        return -1;
    }
    return TokPrec;
}

//! expression
//!   ::= primary binoprhs
//!
static std::unique_ptr<ExprAST> ParseExpression() {
    auto LHS = ParsePrimary();
    if (!LHS) {
        return nullptr;
    }

    return ParseBinOpRHS(0, move(LHS));
}

//! binoprhs
//!   ::= ('+' primary)*
static unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec, unique_ptr<ExprAST> LHS) {
    // If this is a binop, find its precedence.
    while (1) {
        int TokPrec = GetTokPrecedence();

        // If this is a binop that binds at least as tightly as the current binop,
        // consume it, otherwise we are done.
        if (TokPrec < ExprPrec) {
            return LHS;
        }

        // Okay, we know this is a binop.
        int BinOp = CurTok;
        getNextToken();  // eat binop

        // Parse the primary expression after the binary operator.
        auto RHS = ParsePrimary();
        if (!RHS) {
            return nullptr;
        }

        // If BinOp binds less tightly with RHS than the operator after RHS, let
        // the pending operator take RHS as its LHS.
        int NextPrec = GetTokPrecedence();
        if (TokPrec < NextPrec) {

            RHS = ParseBinOpRHS(TokPrec+1, std::move(RHS));
            if (!RHS) {
                return nullptr;
            }

        }

        // Merge LHS/RHS.
        LHS = make_unique<BinaryExprAST>(BinOp, move(LHS), move(RHS));
    }  // loop around to the top of the while loop.
}

//! prototype
//!   ::= id '(' id* ')'
static unique_ptr<PrototypeAST> ParsePrototype() {
    if (CurTok != static_cast<int>(Token::Identifier)) {
        return LogErrorP("Expected function name in prototype");
    }

    std::string FnName = IdentifierStr;
    getNextToken();

    if (CurTok != '(') {
        return LogErrorP("Expected '(' in prototype");
    }

    // Read the list of argument names.
    std::vector<std::string> ArgNames;
    while (getNextToken() == static_cast<int>(Token::Identifier)) {
        ArgNames.push_back(IdentifierStr);
    }

    if (CurTok != ')') {
        return LogErrorP("Expected ')' in prototype");
    }

    // success.
    getNextToken();  // eat ')'.

    return make_unique<PrototypeAST>(FnName, std::move(ArgNames));
}

//! definition ::= 'def' prototype expression
static unique_ptr<FunctionAST> ParseDefinition() {
    getNextToken();  // eat def.
    auto Proto = ParsePrototype();
    if (!Proto) return nullptr;

    if (auto E = ParseExpression()) {
        return make_unique<FunctionAST>(std::move(Proto), std::move(E));
    }
    return nullptr;
}

//! external ::= 'extern' prototype
static std::unique_ptr<PrototypeAST> ParseExtern() {
    getNextToken();  // eat extern.
    return ParsePrototype();
}

//! toplevelexpr ::= expression
static std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
    if (auto E = ParseExpression()) {
        // Make an anonymous proto.
        auto Proto = make_unique<PrototypeAST>("", std::vector<std::string>());
        return make_unique<FunctionAST>(std::move(Proto), std::move(E));
    }
    return nullptr;
}

//===----------------------------------------------------------------------===//
// Top-Level parsing
//===----------------------------------------------------------------------===//

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
static void MainLoop() {
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

int main() {
    // Install standard binary operators.
    // 1 is lowest precedence.
    BinopPrecedence['<'] = 10;
    BinopPrecedence['+'] = 20;
    BinopPrecedence['-'] = 20;
    BinopPrecedence['*'] = 40;  // highest.

    // Prime the first token.
    fprintf(stderr, "ready> ");
    getNextToken();

    // Run the main "interpreter loop" now.
    MainLoop();

    return 0;
}