//
// Created by Markus on 13.07.2016.
//

#include "ast.h"
#include "lexer.h"
#include "parser.h"

#include "helper.h"
using namespace helper;

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
unique_ptr<ExprAST> ParseNumberExpr() {
    auto Result = make_unique<NumberExprAST>(NumVal);
    getNextToken(); // consume the number
    return Result;
}

//! parenexpr ::= '(' expression ')'
unique_ptr<ExprAST> ParseParenExpr() {
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
unique_ptr<ExprAST> ParseIdentifierExpr() {
    string IdName = IdentifierStr;

    getNextToken();  // eat identifier.

    if (CurTok != '(') // Simple variable ref.
        return make_unique<VariableExprAST>(IdName);

    // Call.
    getNextToken();  // eat (
    vector<unique_ptr<ExprAST>> Args;
    if (CurTok != ')') {
        while (1) {
            if (auto Arg = ParseExpression()) {
                Args.push_back(std::move(Arg));
            }
            else {
                return nullptr;
            }

            if (CurTok == ')') {
                break;
            }

            if (CurTok != ',') {
                return LogError("Expected ')' or ',' in argument list");
            }
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
unique_ptr<ExprAST> ParsePrimary() {
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
map<char, int> BinopPrecedence;

//! GetTokPrecedence - Get the precedence of the pending binary operator token.
int GetTokPrecedence() {
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
std::unique_ptr<ExprAST> ParseExpression() {
    auto LHS = ParsePrimary();
    if (!LHS) {
        return nullptr;
    }

    return ParseBinOpRHS(0, move(LHS));
}

//! binoprhs
//!   ::= ('+' primary)*
unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec, unique_ptr<ExprAST> LHS) {
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
unique_ptr<PrototypeAST> ParsePrototype() {
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
unique_ptr<FunctionAST> ParseDefinition() {
    getNextToken();  // eat def.
    auto Proto = ParsePrototype();
    if (!Proto) return nullptr;

    if (auto E = ParseExpression()) {
        return make_unique<FunctionAST>(std::move(Proto), std::move(E));
    }
    return nullptr;
}

//! external ::= 'extern' prototype
std::unique_ptr<PrototypeAST> ParseExtern() {
    getNextToken();  // eat extern.
    return ParsePrototype();
}

//! toplevelexpr ::= expression
std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
    if (auto E = ParseExpression()) {
        // Make an anonymous proto.
        auto Proto = make_unique<PrototypeAST>("", std::vector<std::string>());
        return make_unique<FunctionAST>(std::move(Proto), std::move(E));
    }
    return nullptr;
}
