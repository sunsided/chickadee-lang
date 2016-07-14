//
// Created by Markus on 13.07.2016.
//

#ifndef CHICKADEE_AST_H_H
#define CHICKADEE_AST_H_H

#include <string>
#include <memory>
#include <vector>

#include <llvm/IR/Value.h>

using namespace std;
using namespace llvm;

//! ExprAST - Base class for all expression nodes.
class ExprAST {
public:
    virtual ~ExprAST() {}
    virtual Value *codegen() = 0;
};

//! NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST {
    double _val;

public:
    NumberExprAST(double Val) : _val(Val) {}
    Value *codegen() override;
};

//! VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST {
    string _name;

public:
    VariableExprAST(const string &Name) : _name(Name) {}
    Value *codegen() override;
};

//! BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST {
    char _op;
    unique_ptr<ExprAST> LHS, RHS;

public:
    BinaryExprAST(char op, unique_ptr<ExprAST> LHS, unique_ptr<ExprAST> RHS)
            : _op(op), LHS(move(LHS)), RHS(move(RHS)) {}
    Value *codegen() override;
};

//! CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
    string _callee;
    vector<unique_ptr<ExprAST>> _args;

public:
    CallExprAST(const string &Callee, vector<unique_ptr<ExprAST>> Args)
            : _callee(Callee), _args(std::move(Args)) {}
    Value *codegen() override;
};

//! PrototypeAST - This class represents the "prototype" for a function,
//! which captures its name, and its argument names (thus implicitly the number
//! of arguments the function takes).
class PrototypeAST {
    string _name;
    vector<string> _args;

public:
    PrototypeAST(const string &name, vector<string> Args)
            : _name(name), _args(move(Args)) {}
    Function *codegen();

    const std::string &getName() const { return _name; }
};

//! FunctionAST - This class represents a function definition itself.
class FunctionAST {
    unique_ptr<PrototypeAST> _proto;
    unique_ptr<ExprAST> _body;

public:
    FunctionAST(unique_ptr<PrototypeAST> Proto, unique_ptr<ExprAST> Body)
            : _proto(move(Proto)), _body(move(Body)) {}
    Function *codegen();
};

#endif //CHICKADEE_AST_H_H
