#ifndef WFORMULA_H
#define WFORMULA_H

#include <cstddef>
#include <stack>
#include <vector>
#include <string>
#include <cmath>
//#include <iostream>

#define VECTOR_VARS

#ifdef VECTOR_VARS
#include <Eigen/Dense> 
//using Eigen::ArrayXd;
#define VarType Eigen::ArrayXd
#else
#define VarType double
#endif

/*
The provided expression is translated first into series of commands in postfix order for a simple stack-based
evaluator.

Possible values of CmdType and associated actions:
0  CmdNop: no operation (for debugging)
1  CmdOper: take two top elements from the stack, perform operation @addr on them, push the result
2  CmdFunc: take the top element from the stack, perform function @addr on it, push the result
3  CmdReadConst: push constant @addr to the stack
4  CmdReadVar: push variable @addr to the stack
5  CmdWriteVar: take element from the stack, store it into variable @addr
6  CmdReturn: stop execution, return top of the stack
*/

enum TokenType {
    TokNull = 0,
    TokNumber,
    TokConst,
    TokVar,
    TokFunc,
    TokOper,
    TokUnary,
    TokOpen,
    TokClose,
    TokComma,
    TokEnd,
    TokError
};

struct Token {
    TokenType type;
    std::string string;
    int addr;
    int args;
    Token(TokenType t, std::string s, int a=0) : type(t), string(s), addr(a) {;}
};

struct MyStack {
    double s[256];
    double *ptr = s;
    inline double &top() {return *ptr;}
    inline void pop() {ptr--;}
    inline void push(double val) {*(++ptr)=val;}
    inline int size() {return ptr-s;} 
};
/*
struct MyStack {
    double s[256];
    double *ptr = s+255;
    inline double &top() {return *ptr;}
    inline void pop() {ptr++;}
    inline void push(double val) {*(--ptr)=val;}
    inline int size() {return s-ptr+255;} 
};
*/
struct Cmdaddr{
    unsigned short cmd;
    unsigned short addr;
    Cmdaddr(int c, int a) : cmd(c), addr(a) {;} 
}; 

// aux. structures for debugging
struct assm {
    unsigned short cmd;
    unsigned short addr;
    std::string mnem;
    std::string val;
};

class WFormula
{
public: 
    typedef void (WFormula::*FuncPtr)();

    enum CmdType {
        CmdNop = 0,
        CmdOper,
        CmdFunc,
        CmdReadConst,
        CmdReadVar,
        CmdWriteVar,
        CmdReturn
    };

// Evaluator memory
    std::vector <Cmdaddr> Command; // expression translated to commands in postfix order
    std::vector <double> Const;  // vector of constants
    std::vector <VarType> Var;    // vector of variables
    std::vector <FuncPtr> Func;  // vector of function pointers 
    std::vector <FuncPtr> Oper;  // vector of operator pointers 
    std::stack <VarType> Stack;   // evaluator stack
//    MyStack Stack;

    void Add() {VarType tmp = Stack.top(); Stack.pop(); Stack.top() += tmp;}
    void Sub() {VarType tmp = Stack.top(); Stack.pop(); Stack.top() -= tmp;}
    void Mul() {VarType tmp = Stack.top(); Stack.pop(); Stack.top() *= tmp;}
    void Div() {VarType tmp = Stack.top(); Stack.pop(); Stack.top() /= tmp;}
    void Neg() {Stack.top() = -Stack.top();}
    void Nop() {;}
    void Pow() {VarType tmp = Stack.top(); Stack.pop(); Stack.top() = pow(Stack.top(), tmp);}
    void Pow2() {VarType tmp = Stack.top(); Stack.top() = tmp*tmp;}
    void Pow3() {VarType tmp = Stack.top(); Stack.top() = tmp*tmp*tmp;}

    void Sqrt() {Stack.top() = sqrt(Stack.top());}
    void Exp() {Stack.top() = exp(Stack.top());}
    void Log() {Stack.top() = log(Stack.top());}
    void Sin() {Stack.top() = sin(Stack.top());}
    void Cos() {Stack.top() = cos(Stack.top());}
    void Tan() {Stack.top() = tan(Stack.top());}
    void Asin() {Stack.top() = asin(Stack.top());}
    void Acos() {Stack.top() = acos(Stack.top());}
    void Atan() {Stack.top() = atan(Stack.top());}
//    void Atan2() {double x = Stack.top(); Stack.pop(); Stack.top() = atan2(Stack.top(), x);} //atan2(y,x)

    void Sinh() {Stack.top() = sinh(Stack.top());}
    void Cosh() {Stack.top() = cosh(Stack.top());}
    void Tanh() {Stack.top() = tanh(Stack.top());}
    void Asinh() {Stack.top() = asinh(Stack.top());}
    void Acosh() {Stack.top() = acosh(Stack.top());}
    void Atanh() {Stack.top() = atanh(Stack.top());}

//    void Int() {double t; modf(Stack.top(), &t); Stack.top() = t;}
//    void Frac() {double t; Stack.top() = modf(Stack.top(), &t);}



// scalar and vector versions of these functions have different implementation 
    #ifdef VECTOR_VARS
    void Abs() {Stack.top() = abs(Stack.top());}
    void Max() {VarType tmp = Stack.top(); Stack.pop(); Stack.top() = tmp.max(Stack.top());}
    void Min() {VarType tmp = Stack.top(); Stack.pop(); Stack.top() = tmp.min(Stack.top());}
    #else
    void Abs() {Stack.top() = fabs(Stack.top());}
    void Max() {VarType tmp = Stack.top(); Stack.pop(); Stack.top() = std::max(tmp, Stack.top());}
    void Min() {VarType tmp = Stack.top(); Stack.pop(); Stack.top() = std::min(tmp, Stack.top());}
    #endif    

   // void Gaus();
   // void Pol2();
   // void Pol3();

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

    WFormula();
    ~WFormula() {;}

    bool FindSymbol(std::vector <std::string> &namevec, std::string symbol, size_t *addr);

    size_t AddOperation(std::string name, FuncPtr ptr, std::string mnem, int rank, int args=2);
    size_t AddFunction(std::string name, FuncPtr ptr, std::string mnem, int args=1);
    size_t AddConstant(std::string name, double val);
    size_t AddVariable(std::string name);

    int GetConstCount() const {return ConstName.size();}
    double GetConstant(std::string name);
    double GetConstant(size_t addr);
    VarType GetVariable(std::string name);
    bool SetConstant(std::string name, double val);
    bool SetConstant(size_t addr, double val);
    bool SetVariable(std::string name, VarType val);

    int ParseExpr(std::string expr);
    bool CheckSyntax(Token token);
    Token GetNextToken();
    bool ShuntingYard();

    Cmdaddr MkCmd(int cmd, int addr) {return Cmdaddr(cmd, addr);}

    std::vector<std::string> GetPrg();
    std::vector<std::string> GetConstMap();
    std::vector<std::string> GetVarMap();
    std::vector<std::string> GetOperMap();
    std::vector<std::string> GetFuncMap();

    void VFail(int pos, std::string msg);
    bool Validate();
    VarType Eval();
    VarType Eval(VarType x);
    VarType Eval(VarType x, VarType y);

    std::string GetErrorString() {return ErrorString;}

//    bool SetVar(std::string name, double val); // returns true if var with this name exists

private:
    size_t AddAutoConstant(double val);
    void PruneConstants();

    std::string Expr;
    size_t TokPos = 0; // current token position in Expr
    Token LastToken = Token(TokNull, "");
    size_t CmdPos = 0;
    std::string ErrorString;
    size_t pow2, pow3; // positions of the fast square and cube functions
    size_t neg, nop; // position of the sign inverse and nop functions 
    bool valid = true; // result of the code validity check
    int veclen = 0;         // length of vectors to operate
public:    
    size_t failpos; // position in the code at which validation failed
};

#endif // WFORMULA_H
