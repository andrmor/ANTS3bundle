#include "vformula.h"

#include <algorithm>
#include <iostream>
#include <cstdio>
#include <cmath>

VFormula::VFormula()
{
    // ---- Operations ----
    // for ranks see  https://en.cppreference.com/w/c/language/operator_precedence

    addOperation("!=", &VFormula::NotEqual,  "NEQ", 7);

    addOperation("||", &VFormula::Or,        "OR",   12);
    addOperation("&&", &VFormula::And,       "AND",  11);
    addOperation("!",  &VFormula::Not,       "NOT",  2, 1);

    addOperation("==",  &VFormula::Equal,    "EQ",  7);
    addOperation(">=", &VFormula::GreaterEq, "GE",  6);
    addOperation(">",  &VFormula::Greater,   "GR",  6);
    addOperation("<=", &VFormula::SmallerEq, "SE",  6);
    addOperation("<",  &VFormula::Smaller,   "SM",  6);

    addOperation("+", &VFormula::Add, "ADD", 5);  //+1
    addOperation("-", &VFormula::Sub, "SUB", 5);  //+1
    addOperation("*", &VFormula::Mul, "MUL", 4);  //+1
    addOperation("/", &VFormula::Div, "DIV", 4);  //+1
    addOperation("^", &VFormula::Pow, "POW", 3);

    // unary minus and plus
    NegPos = addOperation("--", &VFormula::Neg, "NEG", 2, 1);
    NopPos = addOperation("++", &VFormula::Nop, "NOP", 2, 1);

    // ---- Functions ----

    Pow2Pos = addFunction("pow2", &VFormula::Pow2, "POW2");
    Pow3Pos = addFunction("pow3", &VFormula::Pow3, "POW3");

    addFunction("pow", &VFormula::Pow, "POW", 2);
    addFunction("abs", &VFormula::Abs, "ABS");       // !!!*** was before addFunction("abs", &VFormula::Exp, "ABS");
    addFunction("sqrt", &VFormula::Sqrt, "SQRT");
    addFunction("exp", &VFormula::Exp, "EXP");
    addFunction("log", &VFormula::Log, "LOG");
    addFunction("sin", &VFormula::Sin, "SIN");
    addFunction("cos", &VFormula::Cos, "COS");
    addFunction("tan", &VFormula::Tan, "TAN");
    addFunction("asin", &VFormula::Asin, "ASIN");
    addFunction("acos", &VFormula::Acos, "ACOS");
    addFunction("atan", &VFormula::Atan, "ATAN");
    addFunction("atan2", &VFormula::Atan2, "ATAN2", 2);

    addFunction("sinh", &VFormula::Sinh, "SINH");
    addFunction("cosh", &VFormula::Cosh, "COSH");
    addFunction("tanh", &VFormula::Tanh, "TANH");
    addFunction("asinh", &VFormula::Asinh, "ASINH");
    addFunction("acosh", &VFormula::Acosh, "ACOSH");
    addFunction("atanh", &VFormula::Atanh, "ATANH");

    addFunction("int", &VFormula::Int, "INT");
    addFunction("frac", &VFormula::Frac, "FRAC");

    addFunction("max", &VFormula::Max, "MAX", 2);
    addFunction("min", &VFormula::Min, "MIN", 2);

    addFunction("gaus", &VFormula::Gaus, "GAUS", 3);
    addFunction("pol2", &VFormula::Pol2, "POL2", 4);

    // ---- Constants ----

    addConstant("pi", M_PI);
}

void VFormula::setVariableNames(const std::vector<std::string> & variables)
{
    VarNames = variables;
}

double VFormula::eval(const std::vector<double> & varValues)
{
    if (varValues.size() != VarNames.size())
    {
        ErrorString = "Mismatch in VFormula variable arrays: names and values";
        return nan("");
    }

    const size_t codelen = Command.size();
    for (size_t i = 0; i < codelen; i++)
    {
        unsigned short cmd  = Command[i].cmd;
        unsigned short addr = Command[i].addr;
        switch (cmd)
        {
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
                Stack.push(varValues[addr]);
                break;
            case CmdReturn:
                double result = Stack.top();
                Stack.pop();
                if (Stack.empty()) return result;
                ErrorString += "\nStack not empty on return!";
                return nan("");
        }
    }
    return nan(""); // can reach this step only on error
}

void VFormula::vFail(int pos, const std::string & msg)
{
    Valid = false;
    FailPos = pos;

    ErrorString = msg;
    ErrorString += "\nFail position: ";
    ErrorString += pos;
}

void VFormula::Not()       {Stack.top() = (Stack.top() == 0 ? 1.0 : 0);}
void VFormula::And()       {double tmp = Stack.top(); Stack.pop(); Stack.top() = Stack.top() && tmp;}
void VFormula::Or()        {double tmp = Stack.top(); Stack.pop(); Stack.top() = Stack.top() || tmp;}
void VFormula::Equal()     {double tmp = Stack.top(); Stack.pop(); Stack.top() = (Stack.top() == tmp);}
void VFormula::NotEqual()  {double tmp = Stack.top(); Stack.pop(); Stack.top() = (Stack.top() != tmp);}
void VFormula::Greater()   {double tmp = Stack.top(); Stack.pop(); Stack.top() = (Stack.top() >  tmp);}
void VFormula::GreaterEq() {double tmp = Stack.top(); Stack.pop(); Stack.top() = (Stack.top() >= tmp);}
void VFormula::Smaller()   {double tmp = Stack.top(); Stack.pop(); Stack.top() = (Stack.top() <  tmp);}
void VFormula::SmallerEq() {double tmp = Stack.top(); Stack.pop(); Stack.top() = (Stack.top() <= tmp);}
void VFormula::Add() {double tmp = Stack.top(); Stack.pop(); Stack.top() += tmp;}
void VFormula::Sub() {double tmp = Stack.top(); Stack.pop(); Stack.top() -= tmp;}
void VFormula::Mul() {double tmp = Stack.top(); Stack.pop(); Stack.top() *= tmp;}
void VFormula::Div() {double tmp = Stack.top(); Stack.pop(); Stack.top() /= tmp;}
void VFormula::Neg() {Stack.top() = -Stack.top();}
void VFormula::Pow() {double tmp = Stack.top(); Stack.pop(); Stack.top() = pow(Stack.top(), tmp);}
void VFormula::Pow2() {double tmp = Stack.top(); Stack.top() = tmp*tmp;}
void VFormula::Pow3() {double tmp = Stack.top(); Stack.top() = tmp*tmp*tmp;}
void VFormula::Abs() {Stack.top() = fabs(Stack.top());}
void VFormula::Sqrt() {Stack.top() = sqrt(Stack.top());}
void VFormula::Exp() {Stack.top() = exp(Stack.top());}
void VFormula::Log() {Stack.top() = log(Stack.top());}
void VFormula::Sin() {Stack.top() = sin(Stack.top());}
void VFormula::Cos() {Stack.top() = cos(Stack.top());}
void VFormula::Tan() {Stack.top() = tan(Stack.top());}
void VFormula::Asin() {Stack.top() = asin(Stack.top());}
void VFormula::Acos() {Stack.top() = acos(Stack.top());}
void VFormula::Atan() {Stack.top() = atan(Stack.top());}
void VFormula::Atan2() {double x = Stack.top(); Stack.pop(); Stack.top() = atan2(Stack.top(), x);}  //atan2(y,x)
void VFormula::Sinh() {Stack.top() = sinh(Stack.top());}
void VFormula::Cosh() {Stack.top() = cosh(Stack.top());}
void VFormula::Tanh() {Stack.top() = tanh(Stack.top());}
void VFormula::Asinh() {Stack.top() = asinh(Stack.top());}
void VFormula::Acosh() {Stack.top() = acosh(Stack.top());}
void VFormula::Atanh() {Stack.top() = atanh(Stack.top());}
void VFormula::Int() {double t; modf(Stack.top(), &t); Stack.top() = t;}
void VFormula::Frac() {double t; Stack.top() = modf(Stack.top(), &t);}
void VFormula::Max() {double tmp = Stack.top(); Stack.pop(); Stack.top() = std::max(tmp, Stack.top());}
void VFormula::Min() {double tmp = Stack.top(); Stack.pop(); Stack.top() = std::min(tmp, Stack.top());}
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

bool VFormula::validate()
{
    Valid = true;
    size_t codelen = Command.size();
    int stkptr = 0;
    bool finished = false;

    for (size_t i=0; i<codelen; i++)
    {
        unsigned short cmd = Command[i].cmd;
        unsigned short addr = Command[i].addr;
        switch (cmd) {
            case CmdOper:
                if (addr >= Oper.size())
                    vFail(i, "Operation out of range");
                stkptr = stkptr - OperArgs[addr] + 1;
                break;
            case CmdFunc:
                if (addr >= Func.size())
                    vFail(i, "Function out of range");
                stkptr = stkptr - FuncArgs[addr] + 1;
                break;
            case CmdReadConst:
                if (addr >= Const.size())
                    vFail(i, "Constant out of range");
                stkptr = stkptr + 1;
                break;
            case CmdReadVar:
                if (addr >= VarNames.size())
                    vFail(i, "Variable out of range");
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
        vFail(-1, std::string("Stack is out of balance by") + std::to_string(stkptr) + "positions");

    return Valid;
}

bool VFormula::parse(const std::string & expr)
{
    Expr = expr;
    TokPos = 0;
    LastToken = Token(TokNull, "");
    Command.clear();
    bool status = shuntingYard();
    if (!status)
    {
        //std::string err = std::string(TokPos-1, ' '); // !!!***
        std::string err = "\nParsing failed at position " + std::to_string(TokPos);
        err += ":\n";
        ErrorString += err;
        std::cout << ErrorString << std::endl;
        return false;
    }
    return true;
}

void VFormula::printPrg()
{
    char buf[32];
    for (auto cmd : Command)
    {
        int c = cmd.cmd;
        int i = cmd.addr;
        sprintf(buf, "%02d:%02d ", c, i);

        if (c == CmdOper)
            std::cout << buf << "\tOpr\t" << OperMnem[i] << std::endl;
        else if (c == CmdFunc)
            std::cout << buf << "\tFun\t" << FuncMnem[i] << std::endl;
        else if (c == CmdReadConst)
        {
            if (ConstNames[i].size() == 0)
                std::cout << buf << "\tCon\t" << Const[i] << std::endl;
            else
                std::cout << buf << "\tCon\t" << ConstNames[i] << "=" << Const[i] << std::endl;
        }
        else if (c == CmdReadVar)
            std::cout << buf << "\tVar\t" << VarNames[i] /*<< "=" << Var[i]*/ << std::endl;
    }
}

void VFormula::printCVMap()
{
    std::cout << "Constants\n";
    for (size_t i=0; i<ConstNames.size(); i++)
        std::cout << ConstNames[i] << " : " << Const[i] << std::endl;
    std::cout << "Variables\n";
    for (size_t i=0; i<VarNames.size(); i++)
        std::cout << VarNames[i] /*<< " : " << Var[i]*/ << std::endl;
}

void VFormula::printOFMap()
{
    std::cout << "Operators\n";
    for (size_t i=0; i<OperName.size(); i++)
        std::cout << OperName[i] << " : " << OperMnem[i] << std::endl;
    std::cout << "Functions\n";
    for (size_t i=0; i<FuncNames.size(); i++)
        std::cout << FuncNames[i] << " : " << FuncMnem[i] << std::endl;
}

VFormula::Token VFormula::getNextToken()
{
// skip spaces
    while (TokPos < Expr.size() && Expr[TokPos] == ' ')
        TokPos++;

    if (TokPos >= Expr.size())
        return Token(TokEnd, "");

    int ch0 = Expr[TokPos]; // fetch the character at the current token position

// parentheses and commas
    if (ch0 == '(')
    {
        TokPos++;
        return Token(TokOpen, "(");
    }
    if (ch0 == ')')
    {
        TokPos++;
        return Token(TokClose, ")");
    }
    if (ch0 == ',')
    {
        TokPos++;
        return Token(TokComma, ",");
    }

// number
    if (std::isdigit(ch0))
    {
        std::size_t len;
        double val = std::stod(Expr.substr(TokPos, std::string::npos), &len);
        int addr = addConstant("", val); // numbers are stored as nameless constants
        TokPos += len;
        return Token(TokNumber, Expr.substr(TokPos-len, len), addr);
    }

// symbol (variable, constant or function name)
    if (std::isalpha(ch0))
    {
        size_t len = 1;
        for (size_t i=TokPos+1; i<Expr.size(); i++)
        {
            int ch = Expr[i];
            if (!isalpha(ch) && !isdigit(ch) && ch!='_')
                break;
            len++;
        }
        std::string symbol = Expr.substr(TokPos, len);
        TokPos += len;

    // now check if it is a known symbol
        size_t addr;
        if (findSymbol(ConstNames, symbol, &addr))
            return Token(TokConst, symbol, addr);

        if (findSymbol(VarNames, symbol, &addr))
            return Token(TokVar, symbol, addr);

        if (findSymbol(FuncNames, symbol, &addr))
        {
            if (Expr[TokPos] != '(')
            {
                TokPos -= len;
                return Token(TokError, std::string("Known function ")+symbol+" without ()");
            }
            return Token(TokFunc, symbol, addr);
        }

        TokPos -= len;
        return Token(TokError, std::string("Unknown symbol: ")+symbol);
    }

// unary minus and plus
    if (ch0 == '-' || ch0 == '+')
    {
        TokenType t = LastToken.type;
        if ( t == TokNull || t == TokOpen || t == TokOper || t==TokComma)
        {
            TokPos++;
            return Token(TokUnary, ch0 == '-' ? "-" : "+", ch0 == '-' ? NegPos : NopPos);
        }
    }

// operators
    for (size_t i=0; i<OperName.size(); i++)
        if (Expr.substr(TokPos, std::string::npos).find(OperName[i]) == 0)
        {
            TokPos += OperName[i].size();
            return Token(TokOper, OperName[i], i);
        }

    return Token(TokError, "Unknown character or character combination");
}

bool VFormula::shuntingYard()
{
// we'll track parentheses level
// it must never get negative and return to zero in the end
    int par_level = 0;

    while (1)
    {
        Token token = getNextToken();
        if (!checkSyntax(token))
        {
            return false;
        }

        if (token.type == TokError)
        {
            ErrorString = token.string;
            return false;
        }

        if (token.type == TokNumber || token.type == TokConst)
        {
        // we have special treatment for the cases of ^2 and ^3
        // to make them process a bit faster
            if (!OpStack.empty() && OpStack.top().string == "^" && Const[token.addr] == 2)
            { // ^2
                Command.push_back(mkCmd(CmdFunc, Pow2Pos));
                OpStack.pop();
            }
            else if (!OpStack.empty() && OpStack.top().string == "^" && Const[token.addr] == 3)
            { // ^3
                Command.push_back(mkCmd(CmdFunc, Pow3Pos));
                OpStack.pop();
            }
            else
            { // in all other cases
                Command.push_back(mkCmd(CmdReadConst, token.addr)); // move to command queue
            }
        }

        else if (token.type == TokVar)
            Command.push_back(mkCmd(CmdReadVar, token.addr)); // move to command queue

        else if (token.type == TokFunc)
        {
            token.args = FuncArgs[token.addr]; // fill correct number of args (should be done in tokenizer?)
            OpStack.push(token); // push to Op stack
        }

        else if (token.type == TokUnary)
        {
            if (token.string == "-")
                OpStack.push(token); // push to Op stack
        }

        else if (token.type == TokOper)
        {
            int rank = OperRank[token.addr];
            while (!OpStack.empty())
            {
                Token op2 = OpStack.top();
                // <=  assuming all operators are left-associative
                if ((op2.type == TokOper && OperRank[op2.addr] <= rank) || op2.type == TokUnary)
                {
                    Command.push_back(mkCmd(CmdOper, op2.addr));
                    OpStack.pop();
                }
                else
                {
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

        else if (token.type == TokClose || token.type == TokComma)
        {
            if (token.type == TokClose)
            {
                par_level--;
                if (par_level < 0)
                {
                    ErrorString = "Extra )";
                    return false;
                }
            }

            while (!OpStack.empty() && OpStack.top().type != TokOpen)
            {
                Command.push_back(mkCmd(CmdOper, OpStack.top().addr));
                OpStack.pop();
            }
            if (OpStack.empty())
            {
                ErrorString = "Mismatched parenthesis";
                return false;
            }

            if (OpStack.top().type == TokOpen) // at this point this should be always true
                OpStack.pop();
            else
            {
                ErrorString = "Parentheses canary: check the parsing code";
                return false;
            }

            if (!OpStack.empty() && OpStack.top().type == TokFunc)
                if (--(OpStack.top().args) == 0)
                {
                    Command.push_back(mkCmd(CmdFunc, OpStack.top().addr));
                    OpStack.pop();
                }

            if (token.type == TokComma) // "," is equivalent to ")("
                OpStack.push(Token(TokOpen, "("));
        }

        else if (token.type == TokEnd)
        {
            if (par_level != 0)
            {
                ErrorString = std::string("Unbalanced ") + std::string(par_level, '(');
                return false;
            }
            break;
        }
        LastToken = token;
    }

    while (!OpStack.empty())
    {
        Command.push_back(mkCmd(CmdOper, OpStack.top().addr));
        OpStack.pop();
    }
    Command.push_back(mkCmd(CmdReturn, 0));
    return true;
}

bool VFormula::checkSyntax(Token token)
{
    TokenType cur = token.type;
    TokenType last = LastToken.type;
    if(cur == TokOper && last == TokOper)
    {
        ErrorString = "Missing Operand";
        return false;
    }
    if ((cur == TokConst || cur == TokVar || cur == TokNumber || cur == TokOpen || cur == TokFunc) &&
        (last == TokConst || last == TokVar || last == TokNumber))
    {
            ErrorString = "Missing Operator";
            return false;
    }
    if ((cur == TokConst || cur == TokVar || cur == TokNumber || cur == TokFunc) &&
        (last == TokClose))
    {
            ErrorString = "Missing Operator";
            return false;
    }
    return true;
}

bool VFormula::findSymbol(std::vector<std::string> & namevec, std::string symbol, size_t *addr)
{
    std::vector <std::string> :: iterator itr;

    itr = std::find(namevec.begin(), namevec.end(), symbol);
    if (itr == namevec.end())
        return false;

    *addr = itr - namevec.begin();
    return true;
}

size_t VFormula::addOperation(std::string name, FuncPtr ptr, std::string mnem, int rank, int args)
{
    OperName.push_back(name);
    OperMnem.push_back(mnem);
    OperRank.push_back(rank);
    OperArgs.push_back(args);
    Oper.push_back(ptr);
    return Oper.size()-1;
}

size_t VFormula::addFunction(std::string name, FuncPtr ptr, std::string mnem, int args)
{
    FuncNames.push_back(name);
    FuncMnem.push_back(mnem);
    FuncArgs.push_back(args);
    Func.push_back(ptr);
    return Func.size()-1;
}

size_t VFormula::addConstant(std::string name, double val)
{
    size_t addr;
    if (name.empty())
    {
        // if automatically generated (empty name)
        std::vector <double> :: iterator itr = std::find(Const.begin(), Const.end(), val);
        if (itr != Const.end()) // if a constant with the same value already exists
            return itr - Const.begin(); // use it
    }
    else if (findSymbol(ConstNames, name, &addr))
    {
        // if the constant with this name already exists - update it
        Const[addr] = val; // !!!***FIX: here was Var[addr] = val;
        return addr;
    }

    // otherwise create a new one
    ConstNames.push_back(name);
    Const.push_back(val);
    return Const.size()-1;
}
