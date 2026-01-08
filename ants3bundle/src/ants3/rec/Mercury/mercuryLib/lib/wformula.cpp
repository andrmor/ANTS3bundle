#include "wformula.h"
#include <algorithm>
//#include <iostream>
#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>
#include <stdexcept>
//#include <chrono>

WFormula::WFormula()
{
    AddOperation("+", &WFormula::Add, "ADD", 5);
    AddOperation("-", &WFormula::Sub, "SUB", 5);
    AddOperation("*", &WFormula::Mul, "MUL", 4);
    AddOperation("/", &WFormula::Div, "DIV", 4);
    AddOperation("^", &WFormula::Pow, "POW", 3);
// unary minus and plus  
    neg = AddOperation("--", &WFormula::Neg, "NEG", 2, 1);
    nop = AddOperation("++", &WFormula::Nop, "NOP", 2, 1);
    
    pow2 = AddFunction("pow2", &WFormula::Pow2, "POW2");
    pow3 = AddFunction("pow3", &WFormula::Pow3, "POW3");

    AddFunction("pow", &WFormula::Pow, "POW", 2);
    AddFunction("abs", &WFormula::Abs, "ABS");
    AddFunction("sqrt", &WFormula::Sqrt, "SQRT");
    AddFunction("exp", &WFormula::Exp, "EXP");
    AddFunction("log", &WFormula::Log, "LOG");    
    AddFunction("sin", &WFormula::Sin, "SIN");
    AddFunction("cos", &WFormula::Cos, "COS");
    AddFunction("tan", &WFormula::Tan, "TAN");
    AddFunction("asin", &WFormula::Asin, "ASIN");
    AddFunction("acos", &WFormula::Acos, "ACOS");
    AddFunction("atan", &WFormula::Atan, "ATAN");
//    AddFunction("atan2", &WFormula::Atan2, "ATAN2", 2);

    AddFunction("sinh", &WFormula::Sinh, "SINH");
    AddFunction("cosh", &WFormula::Cosh, "COSH");
    AddFunction("tanh", &WFormula::Tanh, "TANH");
    AddFunction("asinh", &WFormula::Asinh, "ASINH");
    AddFunction("acosh", &WFormula::Acosh, "ACOSH");
    AddFunction("atanh", &WFormula::Atanh, "ATANH");

//    AddFunction("int", &WFormula::Int, "INT");
//    AddFunction("frac", &WFormula::Frac, "FRAC");

    AddFunction("max", &WFormula::Max, "MAX", 2);
    AddFunction("min", &WFormula::Min, "MIN", 2);

//    AddFunction("gaus", &WFormula::Gaus, "GAUS", 3);
//    AddFunction("pol2", &WFormula::Pol2, "POL2", 4);

    AddConstant("pi", M_PI);

//    AddVariable("x");
//    AddVariable("y", 0.);
//    AddVariable("z", 0.);
}
/*
void WFormula::Gaus()
{
    double sigma = Stack.top(); 
    Stack.pop();
    double x0 = Stack.top(); 
    Stack.pop();
    double t = (Stack.top()-x0)/sigma;
    Stack.top() = 1/(sigma*sqrt(M_PI*2))*exp(-0.5*t*t);
}

void WFormula::Pol2()
{
    double a0 = Stack.top(); 
    Stack.pop();
    double a1 = Stack.top(); 
    Stack.pop();
    double a2 = Stack.top();
    Stack.pop();
    double t = Stack.top();
    Stack.top() = (a2*t+a1)*t+a0;
}
*/
VarType WFormula::Eval(VarType x)
{
    Var[0] = x;
#ifdef VECTOR_VARS
    veclen = x.size();
#endif 
    return Eval();
}

VarType WFormula::Eval(VarType x, VarType y)
{
    Var[0] = x;
    Var[1] = y;
#ifdef VECTOR_VARS
    veclen = x.size();
#endif 
    return Eval();
}

VarType WFormula::Eval()
{
    size_t codelen = Command.size();
    for (size_t i=0; i<codelen; i++) {
        unsigned short cmd = Command[i].cmd;
        unsigned short addr = Command[i].addr;
        switch (cmd) {
            case CmdOper:
                (this->*Oper[addr])();
                break;
            case CmdFunc:
                (this->*Func[addr])();
                break;
            case CmdReadConst:
#ifdef VECTOR_VARS
                Stack.push(VarType::Constant(veclen, Const[addr]));
#else
                Stack.push(Const[addr]);
#endif             
                break;
            case CmdReadVar:
                Stack.push(Var[addr]);
                break;
            case CmdReturn:
                VarType result = Stack.top();
                Stack.pop();
                return result;                         
        }
    }
#ifdef VECTOR_VARS
    return VarType::Constant(veclen, 0.);
#else
    return 0.;
#endif
    // should never reach this spot
    //throw std::runtime_error(std::string("Eval: Unknown command ") + std::to_string(cmd));
}

void WFormula::VFail(int pos, std::string msg)
{
    valid = false;
    failpos = pos;
    ErrorString = msg;
}

bool WFormula::Validate()
{
    valid = true;
    size_t codelen = Command.size();
    int stkptr = 0;
    bool finished = false;

    for (size_t i=0; i<codelen; i++) {
        unsigned short cmd = Command[i].cmd;
        unsigned short addr = Command[i].addr;
        switch (cmd) {
            case CmdOper:
                if (addr >= Oper.size())
                    VFail(i, "Operation out of range");
                stkptr = stkptr - OperArgs[addr] + 1; 
                break;
            case CmdFunc:
                if (addr >= Func.size())
                    VFail(i, "Function out of range");
                stkptr = stkptr - FuncArgs[addr] + 1;
                break;
            case CmdReadConst:
                if (addr >= Const.size())
                    VFail(i, "Constant out of range");
                stkptr = stkptr + 1;
                break;
            case CmdReadVar:
                if (addr >= Var.size())
                    VFail(i, "Variable out of range");
                stkptr = stkptr + 1;
                break;
            case CmdReturn:
                stkptr--;
                finished = true;
                break;                      
        }
        if (finished)
            break;
    }

    if (stkptr != 0)
        VFail(-1, std::string("Stack is out of balance by ") + std::to_string(stkptr) + " position(s)");

    return valid;
}

// parses provided expression
// returns 1024 on success or first error position on failure
int WFormula::ParseExpr(std::string expr)
{
    Expr = expr;
    TokPos = 0;
    LastToken = Token(TokNull, "");
    Command.clear();
    while(!OpStack.empty()) // empty operation stack
        OpStack.pop();
    PruneConstants();
    return ShuntingYard() ? 1024 : TokPos;
}

std::vector<std::string> WFormula::GetPrg()
{
    char buf[32];
    std::vector<std::string> out;
    //std::string tab("\t");
    for (auto cmd : Command) {
        int c = cmd.cmd;
        int i = cmd.addr;
        sprintf(buf, "%02d:%02d ", c, i);

        if (c == CmdOper)
            //std::cout << buf << "\t" << OperMnem[i] << std::endl;
            out.push_back(std::string(buf) + "\t" + OperMnem[i]);
        else if (c == CmdFunc)
            //std::cout << buf << "\tCALL\t" << FuncMnem[i] << std::endl;
            out.push_back(std::string(buf) + "\tCALL\t" + FuncMnem[i]);
        else if (c == CmdReadConst) {
            if (i >= ConstName.size())
                //std::cout << buf << "\tPUSHC\t" << Const[i] << std::endl;
                out.push_back(std::string(buf) + "\tPUSHC\t" + std::to_string(Const[i]));
            else
                //std::cout << buf << "\tPUSHC\t" << ConstName[i] << "=" << Const[i] << std::endl;
                out.push_back(std::string(buf) + "\tPUSHC\t" + ConstName[i] + "=" + std::to_string(Const[i]));
        }
        else if (c == CmdReadVar)
            //std::cout << buf << "\tPUSHV\t" << VarName[i] << std::endl;
            out.push_back(std::string(buf) + "\tPUSHV\t" + VarName[i]);
    }
    return out;
}

std::vector<std::string> WFormula::GetConstMap()
{
    std::vector<std::string> out;

    for (size_t i=0; i<Const.size(); i++) {
        std::string name( i<ConstName.size() ? ConstName[i] : "*");
        out.push_back(name + " : " + std::to_string(Const[i]));
    }
        
    return out;      
}

std::vector<std::string> WFormula::GetVarMap()
{
    std::vector<std::string> out;

    for (size_t i=0; i<VarName.size(); i++)
        out.push_back(VarName[i]);
    return out;     
}

std::vector<std::string> WFormula::GetOperMap()
{
    std::vector<std::string> out;

    for (size_t i=0; i<OperName.size(); i++)
        out.push_back(OperName[i] + " : " + OperMnem[i]);
    return out;
}

std::vector<std::string> WFormula::GetFuncMap()
{
    std::vector<std::string> out;

    for (size_t i=0; i<FuncName.size(); i++)
        out.push_back(FuncName[i] + " : " + FuncMnem[i]);
    return out;
}

Token WFormula::GetNextToken()
{
// skip spaces    
    while (TokPos < Expr.size() && Expr[TokPos] == ' ')
        TokPos++;

    if (TokPos >= Expr.size())
        return Token(TokEnd, "");

    int ch0 = Expr[TokPos]; // fetch the character at the current token position

// parentheses and commas
    if (ch0 == '(') {
        TokPos++;
        return Token(TokOpen, "(");
    }
    if (ch0 == ')') {
        TokPos++;
        return Token(TokClose, ")");
    }
    if (ch0 == ',') {
        TokPos++;
        return Token(TokComma, ",");
    }

// number
    if (std::isdigit(ch0)) { 
        std::size_t len;
        double val = std::stod(Expr.substr(TokPos, std::string::npos), &len);
        int addr = AddAutoConstant(val); // numbers are stored as nameless constants 
        TokPos += len;
        return Token(TokNumber, Expr.substr(TokPos-len, len), addr); 
    }

// symbol (variable, constant or function name)
    if (std::isalpha(ch0)) { 
        size_t len = 1;
        for (size_t i=TokPos+1; i<Expr.size(); i++) {
            int ch = Expr[i];
            if (!isalpha(ch) && !isdigit(ch) && ch!='_') 
                break;
            len++;
        }
        std::string symbol = Expr.substr(TokPos, len);
        TokPos += len;

    // now check if it is a known symbol       
        size_t addr;
        if (FindSymbol(ConstName, symbol, &addr))
            return Token(TokConst, symbol, addr);

        if (FindSymbol(VarName, symbol, &addr))
            return Token(TokVar, symbol, addr);

        if (FindSymbol(FuncName, symbol, &addr)) {
            if (Expr[TokPos] != '(') {
                TokPos -= len;
                return Token(TokError, std::string("Known function ")+symbol+" without ()");
            }
            return Token(TokFunc, symbol, addr);
        }

        TokPos -= len;
        return Token(TokError, std::string("Unknown symbol: ")+symbol);
    }

// unary minus and plus
    if (ch0 == '-' || ch0 == '+') {
        TokenType t = LastToken.type;
        if ( t == TokNull || t == TokOpen || t == TokOper || t==TokComma) {
            TokPos++;
            return Token(TokUnary, ch0 == '-' ? "-" : "+", ch0 == '-' ? neg : nop);
        }
    }    

// operators
    for (size_t i=0; i<OperName.size(); i++) 
        if (Expr.substr(TokPos, std::string::npos).find(OperName[i]) == 0) {
            TokPos += OperName[i].size();
            return Token(TokOper, OperName[i], i);
        }

    return Token(TokError, "Unknown character or character combination");
}

bool WFormula::ShuntingYard()
{
// we'll track parentheses level
// it must never get negative and return to zero in the end
    int par_level = 0;  

    while (1) {
        Token token = GetNextToken();
        if (!CheckSyntax(token)) {
            return false;
        }

        if (token.type == TokError) {
            ErrorString = token.string;
            return false;
        } 

        if (token.type == TokNumber || token.type == TokConst) {
        // we have special treatment for the cases of ^2 and ^3
        // to make them process a bit faster
            if (!OpStack.empty() && OpStack.top().string == "^" && Const[token.addr] == 2) { // ^2
                Command.push_back(MkCmd(CmdFunc, pow2));
                OpStack.pop();
            } else if (!OpStack.empty() && OpStack.top().string == "^" && Const[token.addr] == 3) { // ^3
                Command.push_back(MkCmd(CmdFunc, pow3));
                OpStack.pop();
            } else { // in all other cases
                Command.push_back(MkCmd(CmdReadConst, token.addr)); // move to command queue
            }
        }

        else if (token.type == TokVar) 
            Command.push_back(MkCmd(CmdReadVar, token.addr)); // move to command queue

        else if (token.type == TokFunc) {
            token.args = FuncArgs[token.addr]; // fill correct number of args (should be done in tokenizer?)
            OpStack.push(token); // push to Op stack
        }

        else if (token.type == TokUnary) {
            if (token.string == "-")
                OpStack.push(token); // push to Op stack
        }

        else if (token.type == TokOper) {
            int rank = OperRank[token.addr];
            while (!OpStack.empty()) {
                Token op2 = OpStack.top();
                // <=  assuming all operators are left-associative
                if ((op2.type == TokOper && OperRank[op2.addr] <= rank) || op2.type == TokUnary) {
                    Command.push_back(MkCmd(CmdOper, op2.addr));
                    OpStack.pop();
                } else {
                    LastToken = token;
                    break;
                }
            }
            OpStack.push(token);
        }

        else if (token.type == TokOpen) {
            par_level++;
            OpStack.push(token);
        }

        else if (token.type == TokClose || token.type == TokComma) {
            if (token.type == TokClose) {
                par_level--;
                if (par_level < 0) {
                    ErrorString = "Extra )";
                    return false;                   
                }
            }

            while (!OpStack.empty() && OpStack.top().type != TokOpen) {
                Command.push_back(MkCmd(CmdOper, OpStack.top().addr));
                OpStack.pop();
            }
            if (OpStack.empty()) {
                ErrorString = "Mismatched parenthesis";
                return false;
            }

            if (OpStack.top().type == TokOpen) // at this point this should be always true
                OpStack.pop();
            else {
                ErrorString = "Parentheses canary: check the parsing code";
                return false;                
            }               

            if (!OpStack.empty() && OpStack.top().type == TokFunc)
                if (--(OpStack.top().args) == 0) {
                    Command.push_back(MkCmd(CmdFunc, OpStack.top().addr));
                    OpStack.pop();
            }

            if (token.type == TokComma) // "," is equivalent to ")("
                OpStack.push(Token(TokOpen, "("));    
        }

        else if (token.type == TokEnd) {
            if (par_level != 0) {
                ErrorString = std::string("Unbalanced ") + std::string(par_level, '(');
                return false;    
            }
            break;
        }
        LastToken = token;
    }

    while (!OpStack.empty()) {
        Command.push_back(MkCmd(CmdOper, OpStack.top().addr));
        OpStack.pop();
    }
    Command.push_back(MkCmd(CmdReturn, 0));
    return true;
}

bool WFormula::CheckSyntax(Token token)
{
    TokenType cur = token.type;
    TokenType last = LastToken.type;
    if(cur == TokOper && last == TokOper) {
        ErrorString = "Missing Operand";
        return false;
    }
    if ((cur == TokConst || cur == TokVar || cur == TokNumber || cur == TokOpen || cur == TokFunc) && 
        (last == TokConst || last == TokVar || last == TokNumber)) {
            ErrorString = "Missing Operator";
            return false;
    }
    if ((cur == TokConst || cur == TokVar || cur == TokNumber || cur == TokFunc) && 
        (last == TokClose)) {
            ErrorString = "Missing Operator";
            return false;
    }
    return true;
}

bool WFormula::FindSymbol(std::vector <std::string> &namevec, std::string symbol, size_t *addr)
{
    std::vector <std::string> :: iterator itr;

    itr = std::find(namevec.begin(), namevec.end(), symbol);
    if (itr == namevec.end()) 
        return false;

    *addr = itr-namevec.begin();
    return true;
}

size_t WFormula::AddOperation(std::string name, FuncPtr ptr, std::string mnem, int rank, int args)
{
    OperName.push_back(name);
    OperMnem.push_back(mnem);
    OperRank.push_back(rank);
    OperArgs.push_back(args);
    Oper.push_back(ptr);
    return Oper.size()-1;
}

size_t WFormula::AddFunction(std::string name, FuncPtr ptr, std::string mnem, int args) 
{
    FuncName.push_back(name);
    FuncMnem.push_back(mnem);
    FuncArgs.push_back(args);
    Func.push_back(ptr);
    return Func.size()-1;
}

// two types of constants:
//  * named - these are reusable; they either come preset like pi or created by user via AddConstant()
//  * auto - reset each time the parser runs; used to store the numbers from the formula
//  ConstName.size() gives the address of the first auto constant
size_t WFormula::AddConstant(std::string name, double val)
{
    size_t addr;
    if (FindSymbol(ConstName, name, &addr)) { // if the constant with this name already exists - update it
        Const[addr] = val;
        return addr;
    }
// otherwise create a new one      
    ConstName.push_back(name);
    Const.push_back(val);
    return Const.size()-1;
}

size_t WFormula::AddAutoConstant(double val)
{

    std::vector <double> :: iterator itr = std::find(Const.begin() + ConstName.size(), Const.end(), val);
    if (itr != Const.end()) // if an auto constant with the same value already exists
        return itr-Const.begin(); // use it

// otherwise create a new one      
    Const.push_back(val);
    return Const.size()-1;
}

// remove all automatically generated (i.e. nameless) constants
void WFormula::PruneConstants()
{
    Const.resize(ConstName.size());
}

size_t WFormula::AddVariable(std::string name) 
{
// if the variable with this name already exists - return its address   
    size_t addr;
    if (FindSymbol(VarName, name, &addr)) { 
//        Var[addr] = val;
        return addr;
    }
// otherwise create a new one
    VarName.push_back(name);
    Var.push_back(VarType(0));
    return Var.size()-1;
}

double WFormula::GetConstant(std::string name)
{
    size_t addr;
    return FindSymbol(ConstName, name, &addr) ? Const[addr] : nan("");
}

double WFormula::GetConstant(size_t addr)
{
    if (addr<0 || addr>=Const.size())
        return nan("");
    return Const[addr];    
}

VarType WFormula::GetVariable(std::string name)
{
    size_t addr;
    return FindSymbol(VarName, name, &addr) ? Var[addr] : VarType(0);
//    return FindSymbol(VarName, name, &addr) ? Var[addr] : nan("");   
}

bool WFormula::SetConstant(std::string name, double val)
{
    size_t addr;
    bool status = FindSymbol(ConstName, name, &addr);
    if (status)
        Const[addr] = val;
    return status;     
}

bool WFormula::SetConstant(size_t addr, double val)
{
    if (addr<0 || addr>=Const.size())
        return false;
    Const[addr] = val;
    return true;     
}

bool WFormula::SetVariable(std::string name, VarType val)
{
    size_t addr;
    bool status = FindSymbol(VarName, name, &addr);
    if (status)
        Var[addr] = val;
    return status;    
}


