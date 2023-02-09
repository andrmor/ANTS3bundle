#ifndef VFORMULA_H
#define VFORMULA_H

// https://github.com/vovasolo/vformula

#include <stack>
#include <vector>
#include <string>
#include <cmath>
#include <iostream>

/*
The provided expression is translated first into series of commands in postfix order for a simple stack-based evaluator.

Possible values of CmdType and associated actions:
0  CmdNop: no operation (for debugging)
1  CmdOper: take two top elements from the stack, perform operation @addr on them, push the result
2  CmdFunc: take the top element from the stack, perform function @addr on it, push the result
3  CmdReadConst: push constant @addr to the stack
4  CmdReadVar: push variable @addr to the stack
5  CmdWriteVar: take element from the stack, store it into variable @addr
6  CmdReturn: stop execution, return top of the stack
*/

class VFormula
{
public:
    VFormula();
    virtual ~VFormula() {}

    void setVariables(const std::vector<std::string> & variables);

    double eval(const std::vector<double> & varValues);


    enum TokenType {TokNull = 0, TokNumber, TokConst, TokVar, TokFunc, TokOper, TokUnary, TokOpen, TokClose, TokComma, TokEnd, TokError};
    enum CmdType   {CmdNop = 0, CmdOper, CmdFunc, CmdReadConst, CmdReadVar, CmdWriteVar, CmdReturn};

    struct Token
    {
        TokenType type;
        std::string string;
        int addr;
        int args;
        Token(TokenType t, std::string s, int a=0) : type(t), string(s), addr(a) {}
    };

    struct Cmdaddr
    {
        unsigned short cmd;
        unsigned short addr;
        Cmdaddr(int c, int a) : cmd(c), addr(a) {}
    };

// Evaluator memory
    typedef void (VFormula::*FuncPtr)();
    std::vector <Cmdaddr> Command; // expression translated to commands in postfix order
    std::vector <double>  Const;    // vector of constants
    std::vector <double>  Var;      // vector of variables
    std::vector <FuncPtr> Func;    // vector of function pointers
    std::vector <FuncPtr> Oper;    // vector of operator pointers
    std::stack  <double>  Stack;    // evaluator stack

    void Add() {double tmp = Stack.top(); Stack.pop(); Stack.top() += tmp;}
    void Sub() {double tmp = Stack.top(); Stack.pop(); Stack.top() -= tmp;}
    void Mul() {double tmp = Stack.top(); Stack.pop(); Stack.top() *= tmp;}
    void Div() {double tmp = Stack.top(); Stack.pop(); Stack.top() /= tmp;}
    void Neg() {Stack.top() = -Stack.top();}
    void Nop() {}
    void Pow() {double tmp = Stack.top(); Stack.pop(); Stack.top() = pow(Stack.top(), tmp);}
    void Pow2() {double tmp = Stack.top(); Stack.top() = tmp*tmp;}
    void Pow3() {double tmp = Stack.top(); Stack.top() = tmp*tmp*tmp;}
    void Abs() {Stack.top() = abs(Stack.top());}
    void Sqrt() {Stack.top() = sqrt(Stack.top());}
    void Exp() {Stack.top() = exp(Stack.top());}
    void Log() {Stack.top() = log(Stack.top());}
    void Sin() {Stack.top() = sin(Stack.top());}
    void Cos() {Stack.top() = cos(Stack.top());}
    void Tan() {Stack.top() = tan(Stack.top());}
    void Asin() {Stack.top() = asin(Stack.top());}
    void Acos() {Stack.top() = acos(Stack.top());}
    void Atan() {Stack.top() = atan(Stack.top());}
    void Atan2() {double x = Stack.top(); Stack.pop(); Stack.top() = atan2(Stack.top(), x);} //atan2(y,x)

    void Sinh() {Stack.top() = sinh(Stack.top());}
    void Cosh() {Stack.top() = cosh(Stack.top());}
    void Tanh() {Stack.top() = tanh(Stack.top());}
    void Asinh() {Stack.top() = asinh(Stack.top());}
    void Acosh() {Stack.top() = acosh(Stack.top());}
    void Atanh() {Stack.top() = atanh(Stack.top());}

    void Int() {double t; modf(Stack.top(), &t); Stack.top() = t;}
    void Frac() {double t; Stack.top() = modf(Stack.top(), &t);}

    void Max() {double tmp = Stack.top(); Stack.pop(); Stack.top() = std::max(tmp, Stack.top());}
    void Min() {double tmp = Stack.top(); Stack.pop(); Stack.top() = std::min(tmp, Stack.top());}

    void Gaus();
    void Pol2();
    void Pol3();

// Parser memory
    std::vector <std::string> ConstName; // names of constants: position corresponds to position in Const
    std::vector <std::string> VarName;   // names of variables: position corresponds to position in Var
    std::vector <std::string> FuncName;  // names of functions: position corresponds to position in Func
    std::vector <std::string> FuncMnem;  // function mnemonics: position corresponds to position in Func
    std::vector <int> FuncArgs; // number of arguments to take, position corresponds to position in Func
    std::vector <std::string> OperName;  // names of operations: position corresponds to position in Oper
    std::vector <std::string> OperMnem;  // operation mnemonics: position corresponds to position in Oper
    std::vector <int> OperRank;  // operation priorities (less is higher): position corresponds to position in Oper
    std::vector <int> OperArgs;  // number of arguments to take, position corresponds to position in Oper
    std::stack <Token> OpStack;  // parser stack

    bool FindSymbol(std::vector <std::string> &namevec, std::string symbol, size_t *addr);

    size_t AddOperation(std::string name, FuncPtr ptr, std::string mnem, int rank, int args=2);
    size_t AddFunction(std::string name, FuncPtr ptr, std::string mnem, int args=1);
    size_t AddConstant(std::string name, double val);
    size_t AddVariable(std::string name, double val);

    double GetConstant(std::string name);
    double GetVariable(std::string name);
    bool SetConstant(std::string name, double val);
    bool SetVariable(std::string name, double val);

    bool ParseExpr(std::string expr);
    bool CheckSyntax(Token token);
    Token GetNextToken();
    bool ShuntingYard();

    Cmdaddr MkCmd(int cmd, int addr) {return Cmdaddr(cmd, addr);}

    void PrintCVMap();
    void PrintOFMap();
    void PrintPrg();

    void VFail(int pos, std::string msg);
    bool Validate();
    double Eval();
    /*
    double Eval(double x)
    {
        Var[0] = x;
        return Eval();
    }
    */

    size_t GetFailPos() const {return failpos;}
    std::string GetErrorString() {return ErrorString;}

private:
    std::string Expr;
    bool        valid = true; // result of the code validity check
    size_t      TokPos = 0; // current token position in Expr
    Token       LastToken = Token(TokNull, "");
    size_t      CmdPos = 0;
    size_t      pow2, pow3; // positions of the fast square and cube functions
    size_t      neg, nop; // position of the sign inverse and nop functions
    size_t      failpos; // position in the code at which validation failed
    std::string ErrorString;
};

#endif // VFORMULA_H
