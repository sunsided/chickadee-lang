//
// Created by Markus on 13.07.2016.
//

#ifndef CHICKADEE_AST_H_H
#define CHICKADEE_AST_H_H

#include <string>
#include <memory>
#include <vector>

using namespace std;

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

#endif //CHICKADEE_AST_H_H
