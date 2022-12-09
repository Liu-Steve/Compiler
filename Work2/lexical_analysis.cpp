#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <iomanip>
using namespace std;

typedef struct st1
{
    string name;
} ID;

typedef union un1
{
    unsigned long long i;
    double f;
} CONST_VAL;

typedef struct st2
{
    bool isInt;
    CONST_VAL val;
} CT;

enum STA
{
    ERR_GEN = -1,   // 通用错误: 未支持的字符
    ERR_POINT = -2, // 出现意外的小数点
    ERR_CONST = -3, // 不合法的常数格式
    T = 0x1000,     // 代码结束
    S = 0,          // 开始状态
    A,              // 标识符
    B1,             // 常量整数部分，1-9
    B2,             // 常量整数部分，0
    B3,             // 发现小数点
    B4,             // 常量小数部分
    C,              // 运算符
    D               // 分隔符
};

struct CharSetException : public exception
{
    const char *what() const throw()
    {
        return "CharSetException: Not an ASCII char.";
    }
};

struct GeneralException : public exception
{
    const char *what() const throw()
    {
        return "GeneralException: Char not support.";
    }
};

struct PointException : public exception
{
    const char *what() const throw()
    {
        return "PointException: Unexpected point '.' recognized.";
    }
};

struct ConstException : public exception
{
    const char *what() const throw()
    {
        return "ConstException: Illegal const format.";
    }
};

struct NoExistOperatorException : public exception
{
    string op;

    NoExistOperatorException(string str)
    {
        op = str;
    }

    const char *what() const throw()
    {
        return ("NoExistOperatorException: No operator called \"" + op + "\".")
            .c_str();
    }
};

struct NoExistSeparatorException : public exception
{
    string sp;

    NoExistSeparatorException(string str)
    {
        sp = str;
    }

    const char *what() const throw()
    {
        return ("NoExistSeparatorException: No separator called \"" + sp + "\".")
            .c_str();
    }
};

const int STA_CNT = 8;

// 表
string target;
vector<pair<int, int>> tokens;
vector<ID> identifiers;
vector<CT> consts;
const vector<string> operators = {
    ":=", "+", "-", "*", "/",
    "=", ">", "<", ">=", "<=", "!=",
    "&", "|", "!"};
const vector<string> separators = {
    "{", "}", "(", ")", ";"};
const vector<string> keywords = {
    "bool", "char", "short", "int", "long", "float", "double",
    "if", "else", "while", "function", "true", "false"};

const vector<char> opChar = {
    ':', '+', '-', '*', '/', '=', '>', '<', '!', '&', '|'};
const vector<char> spChar = {
    '{', '}', '(', ')', ';'};
const vector<char> blankChar = {
    ' ', '\t', '\r', '\n'};

class FA
{
private:
    stringstream letterBuf;
    int st = 0;
    int stTable[STA_CNT][128];

public:
    string letter;
    FA()
    {
        // 先把所有状态标记为错误状态
        memset(stTable, -1, sizeof(stTable));
        // 把#标为结束状态
        for (int i = 0; i < STA_CNT; ++i)
            stTable[i]['#'] = T;
        // 把.标上异常，常量识别时会更改
        for (int i = 0; i < STA_CNT; ++i)
            stTable[i]['.'] = ERR_POINT;
        // S 开始状态
        for (char i = 'a'; i <= 'z'; ++i)
            stTable[S][i] = A;
        for (char i = 'A'; i <= 'Z'; ++i)
            stTable[S][i] = A;
        for (char i = '1'; i <= '9'; ++i)
            stTable[S][i] = B1;
        stTable[S]['0'] = B2;
        for (char i : opChar)
            stTable[S][i] = C;
        for (char i : spChar)
            stTable[S][i] = D;
        for (char i : blankChar)
            stTable[S][i] = S;
        // 识别结束标志 A 标识符 B 常量 C 运算符 D 分隔符
        vector<STA> stas_start = {A, B1, B2, B4, C, D};
        for (STA sta : stas_start)
        {
            for (char i : opChar)
                stTable[sta][i] = S;
            for (char i : spChar)
                stTable[sta][i] = S;
            for (char i : blankChar)
                stTable[sta][i] = S;
        }
        for (char i = 'a'; i <= 'z'; ++i)
            stTable[C][i] = S;
        for (char i = 'A'; i <= 'Z'; ++i)
            stTable[C][i] = S;
        for (char i = '0'; i <= '9'; ++i)
            stTable[C][i] = S;
        for (char i = 'a'; i <= 'z'; ++i)
            stTable[D][i] = S;
        for (char i = 'A'; i <= 'Z'; ++i)
            stTable[D][i] = S;
        for (char i = '0'; i <= '9'; ++i)
            stTable[D][i] = S;
        // A
        for (char i = 'a'; i <= 'z'; ++i)
            stTable[A][i] = A;
        for (char i = 'A'; i <= 'Z'; ++i)
            stTable[A][i] = A;
        for (char i = '0'; i <= '9'; ++i)
            stTable[A][i] = A;
        // B
        // B Const format exception
        vector<STA> stas_const = {B1, B2, B3, B4};
        for (STA sta : stas_const)
        {
            for (char i = 'a'; i <= 'z'; ++i)
                stTable[sta][i] = ERR_CONST;
            for (char i = 'A'; i <= 'Z'; ++i)
                stTable[sta][i] = ERR_CONST;
        }
        for (char i = '0'; i <= '9'; ++i)
            stTable[B2][i] = ERR_CONST;
        for (char i : opChar)
            stTable[B3][i] = ERR_CONST;
        for (char i : spChar)
            stTable[B3][i] = ERR_CONST;
        for (char i : blankChar)
            stTable[B3][i] = ERR_CONST;
        stTable[B3]['#'] = ERR_CONST;
        // B 其他转换
        for (char i = '0'; i <= '9'; ++i)
            stTable[B1][i] = B1;
        stTable[B1]['.'] = B3;
        stTable[B2]['.'] = B3;
        for (char i = '0'; i <= '9'; ++i)
            stTable[B3][i] = B4;
        for (char i = '0'; i <= '9'; ++i)
            stTable[B4][i] = B4;
        // C
        for (char i : opChar)
            stTable[C][i] = C;
    }

    int update(char ch)
    {
        if (ch >= 128)
            throw CharSetException();
        int newSt = stTable[st][ch];
        int retCode = S; // 未识别完成或者识别结果为空
        switch (newSt)
        {
        case T:
        case S:
            letter = letterBuf.str();
            letterBuf.str(string());
            retCode = st;
            break;
        case ERR_GEN:
            throw GeneralException();
            break;
        case ERR_POINT:
            throw PointException();
            break;
        case ERR_CONST:
            throw ConstException();
            break;
        default:
            letterBuf << ch;
            break;
        }
        st = newSt;
        return retCode;
    }
};

int getKeyword(string &str)
{
    int kwLen = keywords.size();
    for (int i = 0; i < kwLen; ++i)
        if (keywords[i] == str)
            return i;
    return -1;
}

pair<int, int> updateIdentifierOrKeyword(string &str)
{
    int id = getKeyword(str);
    if (id != -1)
        return {5, id};
    int idLen = identifiers.size();
    for (int i = 0; i < idLen; ++i)
        if (identifiers[i].name == str)
            return {1, i};
    ID newId;
    newId.name = str;
    identifiers.emplace_back(newId);
    return {1, idLen};
}

int updateConstInt(string &str)
{
    int ctLen = consts.size();
    unsigned long long val = stoull(str);
    for (int i = 0; i < ctLen; ++i)
        if (consts[i].isInt && consts[i].val.i == val)
            return i;
    CT newCt;
    newCt.isInt = true;
    newCt.val.i = val;
    consts.emplace_back(newCt);
    return ctLen;
}

int updateConstFloat(string &str)
{
    int ctLen = consts.size();
    double val = stod(str);
    for (int i = 0; i < ctLen; ++i)
        if (!consts[i].isInt && consts[i].val.f == val)
            return i;
    CT newCt;
    newCt.isInt = false;
    newCt.val.f = val;
    consts.emplace_back(newCt);
    return ctLen;
}

int getOperator(string &str)
{
    int opLen = operators.size();
    for (int i = 0; i < opLen; ++i)
        if (operators[i] == str)
            return i;
    throw NoExistOperatorException(str);
}

int getSeparator(string &str)
{
    int spLen = separators.size();
    for (int i = 0; i < spLen; ++i)
        if (separators[i] == str)
            return i;
    throw NoExistSeparatorException(str);
}

/**
 * 词法分析程序
 * 返回值：
 *  0 用户程序无词法错误
 *  1 用户程序存在词法错误
 */
int lex()
{
    long long row = 1;
    long long column = 0;
    stringstream program;
    FA fa;
    try
    {
        for (char ch : target)
        {
            // 统计行列
            if (ch == '\n')
            {
                row++;
                column = 0;
                program.str(string());
            }
            else if (ch == '\t')
            {
                column += 4;
                program << ch;
            }
            else
            {
                column++;
                program << ch;
            }
            // 更新自动机状态
            int recognize = fa.update(ch);
            string letter = "";
            if (recognize)
            {
                letter = fa.letter;
                fa.update(ch); // 重喂ch，保证FA在正确的状态下
            }
            // 添加token
            int type;
            int val;
            switch (recognize)
            {
            case S:
                continue;
            case A:
                tokens.emplace_back(updateIdentifierOrKeyword(letter));
                break;
            case B1:
            case B2:
                type = 2;
                val = updateConstInt(letter);
                tokens.emplace_back(type, val);
                break;
            case B3:
                throw ConstException(); // 不应该发生
            case B4:
                type = 2;
                val = updateConstFloat(letter);
                tokens.emplace_back(type, val);
                break;
            case C:
                type = 3;
                val = getOperator(letter);
                tokens.emplace_back(type, val);
                break;
            case D:
                type = 4;
                val = getSeparator(letter);
                tokens.emplace_back(type, val);
                break;

            default:
                throw GeneralException(); // 不应该发生
            }
        }
    }
    catch (exception &e)
    {
        cerr << "[lexical ERROR]\t" << e.what() << endl;
        cerr << ">>>\tHappened at Row: " << row << ", Column: " << column << endl;
        cerr << setw(8) << row;
        cerr << '|' << program.str() << endl;
        cerr << setw(9) << '|' << setw(column) << '^' << endl;
        return -1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    string srcFilePath = "./src/main.steve";
    string outFilePath;

    if (argc < 2)
    {
        clog << "WARN:\tNeed 1 argument to indentify source file." << endl;
        clog << "WARN:\tUsing default target file: " + srcFilePath << endl;
    }
    else if (argc > 3)
    {
        cerr << "ERROR:\tToo many arguments." << endl;
        return -1;
    }
    else
    {
        srcFilePath.assign(argv[1]);
        clog << "INFO:\tTarget file: " + srcFilePath << endl;
    }

    if (argc < 3)
    {
        outFilePath = srcFilePath + ".lex";
        clog << "WARN:\tNeed 1 argument to indentify lex file." << endl;
        clog << "WARN:\tUsing default lex file: " + outFilePath << endl;
    }
    else
    {
        outFilePath.assign(argv[2]);
        clog << "INFO:\tLex file: " + outFilePath << endl;
    }

    // 读源程序文件
    ifstream sourceFile;
    sourceFile.open(srcFilePath);
    if (sourceFile.fail())
    {
        cerr << "ERROR:\tFailed to open file \"" << srcFilePath << "\"" << endl;
        return -1;
    }
    try
    {
        stringstream buf;
        buf << sourceFile.rdbuf() << '#';
        target = buf.str();
    }
    catch (exception &e)
    {
        cerr << "ERROR:\t" << e.what() << endl;
        sourceFile.close();
        return -1;
    }

    // 进行词法分析
    int retCode = lex();
    sourceFile.close();
    if (retCode)
        return 1; // 用户程序存在错误

    // 输出tokens以及符号表，常量表
    ofstream outFile(outFilePath);
    if (outFile.fail())
    {
        cerr << "ERROR:\tFailed to open file \"" << srcFilePath << ".lex\"" << endl;
        return -1;
    }

    try
    {
        outFile << "[TOKEN]" << endl;
        int tokenLen = tokens.size();
        for (int i = 0; i < tokenLen; ++i)
        {
            int type = tokens[i].first;
            int val = tokens[i].second;
            switch (type)
            {
            case 1:
                outFile << setw(16) << left << identifiers[val].name;
                break;
            case 2:
                if (consts[val].isInt)
                    outFile << setw(16) << left << consts[val].val.i;
                else
                    outFile << setw(16) << left << consts[val].val.f;
                break;
            case 3:
                outFile << setw(16) << left << operators[val];
                break;
            case 4:
                outFile << setw(16) << left << separators[val];
                break;
            case 5:
                outFile << setw(16) << left << keywords[val];
                break;
            default:
                break;
            }
            outFile << "<" << tokens[i].first << ", " << tokens[i].second << '>' << endl;
        }

        outFile << endl << "[IDENTIFIER]" << endl;
        int idLen = identifiers.size();
        for(int i = 0; i < idLen; ++i)
        {
            outFile << setw(4) << left << i;
            outFile << identifiers[i].name << endl;
        }

        outFile << endl << "[CONST]" << endl;
        int ctLen = consts.size();
        for(int i = 0; i < ctLen; ++i)
        {
            outFile << setw(4) << left << i;
            if (consts[i].isInt)
                outFile << "INTEGER     " << consts[i].val.i << endl;
            else
                outFile << "FLOAT       " << consts[i].val.f << endl;
        }
    }
    catch (exception &e)
    {
        cerr << "ERROR:\t" << e.what() << endl;
        outFile.close();
        return -1;
    }

    outFile.close();
    return 0;
}