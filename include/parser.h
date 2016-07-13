//
// Created by Markus on 13.07.2016.
//

#ifndef CHICKADEE_PARSER_H
#define CHICKADEE_PARSER_H

#include <map>
#include "ast.h"


/// BinOpPrecedence - This holds the precedence for each binary operator that is
/// defined.
extern map<char, int> BinOpPrecedence;
int GetTokPrecedence();

unique_ptr<ExprAST> LogError(const char *Str);
unique_ptr<PrototypeAST> LogErrorP(const char *Str);
unique_ptr<ExprAST> ParseNumberExpr();
unique_ptr<ExprAST> ParseParenExpr();
unique_ptr<ExprAST> ParseIdentifierExpr();
unique_ptr<ExprAST> ParsePrimary();
unique_ptr<FunctionAST> ParseDefinition();
unique_ptr<PrototypeAST> ParseExtern();
unique_ptr<ExprAST> ParseExpression();
unique_ptr<ExprAST> ParseBinOpRHS(int expressionPrecedence, unique_ptr<ExprAST> LHS);
unique_ptr<PrototypeAST> ParsePrototype();
unique_ptr<FunctionAST> ParseDefinition();
unique_ptr<PrototypeAST> ParseExtern();
unique_ptr<FunctionAST> ParseTopLevelExpr();

#endif //CHICKADEE_PARSER_H
