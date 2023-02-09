#include "vformula.h"

#include <algorithm>
#include <iostream>
#include <cstdio>
#include <chrono>

VFormula::VFormula()
{
    AddOperation("+", &VFormula::Add, "ADD", 5);
    AddOperation("-", &VFormula::Sub, "SUB", 5);
    AddOperation("*", &VFormula::Mul, "MUL", 4);
    AddOperation("/", &VFormula::Div, "DIV", 4);
    AddOperation("^", &VFormula::Pow, "POW", 3);
// unary minus and plus
    neg = AddOperation("--", &VFormula::Neg, "NEG", 2, 1);
    nop = AddOperation("++", &VFormula::Nop, "NOP", 2, 1);

    pow2 = AddFunction("pow2", &VFormula::Pow2, "POW2");
    pow3 = AddFunction("pow3", &VFormula::Pow3, "POW3");

    AddFunction("pow", &VFormula::Pow, "POW", 2);
    AddFunction("abs", &VFormula::Exp, "ABS");
    AddFunction("sqrt", &VFormula::Sqrt, "SQRT");
    AddFunction("exp", &VFormula::Exp, "EXP");
    AddFunction("log", &VFormula::Log, "LOG");
    AddFunction("sin", &VFormula::Sin, "SIN");
    AddFunction("cos", &VFormula::Cos, "COS");
    AddFunction("tan", &VFormula::Tan, "TAN");
    AddFunction("asin", &VFormula::Asin, "ASIN");
    AddFunction("acos", &VFormula::Acos, "ACOS");
    AddFunction("atan", &VFormula::Atan, "ATAN");
    AddFunction("atan2", &VFormula::Atan2, "ATAN2", 2);

    AddFunction("sinh", &VFormula::Sinh, "SINH");
    AddFunction("cosh", &VFormula::Cosh, "COSH");
    AddFunction("tanh", &VFormula::Tanh, "TANH");
    AddFunction("asinh", &VFormula::Asinh, "ASINH");
    AddFunction("acosh", &VFormula::Acosh, "ACOSH");
    AddFunction("atanh", &VFormula::Atanh, "ATANH");

    AddFunction("int", &VFormula::Int, "INT");
    AddFunction("frac", &VFormula::Frac, "FRAC");

    AddFunction("max", &VFormula::Max, "MAX", 2);
    AddFunction("min", &VFormula::Min, "MIN", 2);

    AddFunction("gaus", &VFormula::Gaus, "GAUS", 3);
    AddFunction("pol2", &VFormula::Pol2, "POL2", 4);

    AddConstant("pi", M_PI);

    AddVariable("x", 0.);
    AddVariable("y", 0.);
    AddVariable("z", 0.);
}

void VFormula::Gaus()
{
    double sigma = Stack.top();
    Stack.pop();
    double x0 = Stack.top();
    Stack.pop();
    double t = (Stack.top()-x0)/sigma;
    Stack.top() = 1/(sigma*sqrt(M_PI*2))*exp(-0.5*t*t);
}

void VFormula::Pol2()
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

double VFormula::Eval(double x)
{
    Var[0] = x;
    return Eval();
}

double VFormula::Eval()
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
                Stack.push(Const[addr]);
                break;
            case CmdReadVar:
                Stack.push(Var[addr]);
                break;
            case CmdReturn:
                double result = Stack.top();
                Stack.pop();
                return result;
        }
    }
    return nan(""); // should never reach this spot
}

void VFormula::VFail(int pos, std::string msg)
{
    valid = false;
    failpos = pos;
    ErrorString = msg;
}

bool VFormula::Validate()
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
        VFail(-1, std::string("Stack is out of balance by") + std::to_string(stkptr) + "positions");

    return valid;
}

bool VFormula::ParseExpr(std::string expr)
{
    Expr = expr;
    TokPos = 0;
    LastToken = Token(TokNull, "");
    Command.clear();
    bool status = ShuntingYard();
    if (!status) {
        std::cout << std::string(TokPos-1, ' ') << "^" << std::endl;
        std::cout << "Parsing failed at position " << TokPos << ": ";
        std::cout << ErrorString << std::endl;
        return false;
    }
    return true;
}

void VFormula::PrintPrg()
{
    char buf[32];
    for (auto cmd : Command) {
        int c = cmd.cmd;
        int i = cmd.addr;
        sprintf(buf, "%02d:%02d ", c, i);

        if (c == CmdOper)
            std::cout << buf << "\tOpr\t" << OperMnem[i] << std::endl;
        else if (c == CmdFunc)
            std::cout << buf << "\tFun\t" << FuncMnem[i] << std::endl;
        else if (c == CmdReadConst) {
            if (ConstName[i].size() == 0)
                std::cout << buf << "\tCon\t" << Const[i] << std::endl;
            else
                std::cout << buf << "\tCon\t" << ConstName[i] << "=" << Const[i] << std::endl;
        }
        else if (c == CmdReadVar)
            std::cout << buf << "\tVar\t" << VarName[i] << "=" << Var[i] << std::endl;
    }
}

void VFormula::PrintCVMap()
{
    std::cout << "Constants\n";
    for (size_t i=0; i<ConstName.size(); i++)
        std::cout << ConstName[i] << " : " << Const[i] << std::endl;
    std::cout << "Variables\n";
    for (size_t i=0; i<VarName.size(); i++)
        std::cout << VarName[i] << " : " << Var[i] << std::endl;
}

void VFormula::PrintOFMap()
{
    std::cout << "Operators\n";
    for (size_t i=0; i<OperName.size(); i++)
        std::cout << OperName[i] << " : " << OperMnem[i] << std::endl;
    std::cout << "Functions\n";
    for (size_t i=0; i<FuncName.size(); i++)
        std::cout << FuncName[i] << " : " << FuncMnem[i] << std::endl;
}

Token VFormula::GetNextToken()
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
        int addr = AddConstant("", val); // numbers are stored as nameless constants
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

bool VFormula::ShuntingYard()
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

bool VFormula::CheckSyntax(Token token)
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

bool VFormula::FindSymbol(std::vector <std::string> &namevec, std::string symbol, size_t *addr)
{
    std::vector <std::string> :: iterator itr;

    itr = std::find(namevec.begin(), namevec.end(), symbol);
    if (itr == namevec.end())
        return false;

    *addr = itr-namevec.begin();
    return true;
}

size_t VFormula::AddOperation(std::string name, FuncPtr ptr, std::string mnem, int rank, int args)
{
    OperName.push_back(name);
    OperMnem.push_back(mnem);
    OperRank.push_back(rank);
    OperArgs.push_back(args);
    Oper.push_back(ptr);
    return Oper.size()-1;
}

size_t VFormula::AddFunction(std::string name, FuncPtr ptr, std::string mnem, int args)
{
    FuncName.push_back(name);
    FuncMnem.push_back(mnem);
    FuncArgs.push_back(args);
    Func.push_back(ptr);
    return Func.size()-1;
}

size_t VFormula::AddConstant(std::string name, double val)
{
    size_t addr;
    if (name.size() == 0) { // if automatically generated (empty name)
        std::vector <double> :: iterator itr = std::find(Const.begin(), Const.end(), val);
        if (itr != Const.end()) // if a constant with the same value already exists
            return itr-Const.begin(); // use it
    } else if (FindSymbol(ConstName, name, &addr)) { // if the constant with this name already exists - update it
        Var[addr] = val;
        return addr;
    }
// otherwise create a new one
    ConstName.push_back(name);
    Const.push_back(val);
    return Const.size()-1;
}

size_t VFormula::AddVariable(std::string name, double val)
{
// if the variable with this name already exists - update it
    size_t addr;
    if (FindSymbol(VarName, name, &addr)) {
        Var[addr] = val;
        return addr;
    }
// otherwise create a new one
    VarName.push_back(name);
    Var.push_back(val);
    return Var.size()-1;
}

double VFormula::GetConstant(std::string name)
{
    size_t addr;
    return FindSymbol(ConstName, name, &addr) ? Const[addr] : nan("");
}

double VFormula::GetVariable(std::string name)
{
    size_t addr;
    return FindSymbol(VarName, name, &addr) ? Var[addr] : nan("");
}

bool VFormula::SetConstant(std::string name, double val)
{
    size_t addr;
    bool status = FindSymbol(ConstName, name, &addr);
    if (status)
        Const[addr] = val;
    return status;
}

bool VFormula::SetVariable(std::string name, double val)
{
    size_t addr;
    bool status = FindSymbol(VarName, name, &addr);
    if (status)
        Var[addr] = val;
    return status;
}
