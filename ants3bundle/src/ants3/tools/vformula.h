#ifndef VFORMULA_H
#define VFORMULA_H

// https://github.com/vovasolo/vformula

#include <stack>
#include <vector>
#include <string>

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
    VFormula(const VFormula &) = default;
    virtual ~VFormula() {}

    void   setVariableNames(const std::vector<std::string> & variables);
    bool   parse(const std::string & expr);
    bool   validate();

    bool   isValidated() const {return Valid;}
    double eval(const std::vector<double> & varValues);

    void   printCVMap();
    void   printOFMap();
    void   printPrg();

    std::string ErrorString;

protected:
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

    typedef void (VFormula::*FuncPtr)();

    // Evaluator memory
    std::vector <Cmdaddr> Command; // expression translated to commands in postfix order
    std::vector <double>  Const;    // vector of constants
    std::vector <double>  VarLocal;      // vector of variables
    std::vector <FuncPtr> Func;    // vector of function pointers
    std::vector <FuncPtr> Oper;    // vector of operator pointers
    std::stack  <double>  Stack;    // evaluator stack

    // Parser memory
    std::vector<std::string> ConstNames; // names of constants: position corresponds to position in Const
    std::vector<std::string> VarNames;   // names of variables: position corresponds to position in Var
    std::vector<std::string> FuncNames;  // names of functions: position corresponds to position in Func
    std::vector<std::string> FuncMnem;   // function mnemonics: position corresponds to position in Func
    std::vector<int>         FuncArgs;   // number of arguments to take, position corresponds to position in Func
    std::vector<std::string> OperName;   // names of operations: position corresponds to position in Oper
    std::vector<std::string> OperMnem;   // operation mnemonics: position corresponds to position in Oper
    std::vector<int>         OperRank;   // operation priorities (less is higher): position corresponds to position in Oper
    std::vector<int>         OperArgs;   // number of arguments to take, position corresponds to position in Oper
    std::stack<Token>        OpStack;    // parser stack

    bool    findSymbol(std::vector<std::string> & namevec, std::string symbol, size_t * addr);

    size_t  addOperation(std::string name, FuncPtr ptr, std::string mnem, int rank, int args=2);
    size_t  addFunction(std::string name, FuncPtr ptr, std::string mnem, int args=1);
    size_t  addConstant(std::string name, double val);
    bool    checkSyntax(Token token);
    Token   getNextToken();
    bool    shuntingYard();
    Cmdaddr mkCmd(int cmd, int addr) {return Cmdaddr(cmd, addr);}
    void    vFail(int pos, const std::string & msg);

    std::string Expr;
    bool        Valid = true;                   // result of the code validity check  !!!*** bad idea to have it by default true!
    size_t      TokPos = 0;                     // current token position in Expr
    Token       LastToken = Token(TokNull, "");
    size_t      CmdPos = 0;
    size_t      Pow2Pos, Pow3Pos;               // positions of the fast square and cube functions
    size_t      NegPos, NopPos;                 // position of the sign inverse and nop functions
    size_t      FailPos;                        // position in the code at which validation failed

    // --- VFormula methods ---
    void Equal();
    void NotEqual();
    void Greater();
    void GreaterEq();
    void Smaller();
    void SmallerEq();

    void Add();
    void Sub();
    void Mul();
    void Div();
    void Neg();
    void Nop() {}
    void Pow();
    void Pow2();
    void Pow3();
    void Abs();
    void Sqrt();
    void Exp();
    void Log();
    void Sin();
    void Cos();
    void Tan();
    void Asin();
    void Acos();
    void Atan();
    void Atan2();

    void Sinh();
    void Cosh();
    void Tanh();
    void Asinh();
    void Acosh();
    void Atanh();

    void Int();
    void Frac();

    void Max();
    void Min();

    void Gaus();
    void Pol2();
    // ---
};

#endif // VFORMULA_H
