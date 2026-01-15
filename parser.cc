#include "parser.h"
#include "lexer.h"
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <cassert>

using namespace std;

//---------------------------------------
// Basic Helpers
//---------------------------------------
Parser::Parser(LexicalAnalyzer &lexer) : lexer(lexer) {}

void Parser::syntaxError() {
    cout << "SYNTAX ERROR !!!!!&%!!" << endl;
    exit(1);
}

Token Parser::getNextToken() {
    return lexer.GetToken();
}

Token Parser::peekToken(int k) {
    return lexer.peek(k);
}

void Parser::expect(TokenType expected) {
    Token t = getNextToken();
    if (t.token_type != expected)
        syntaxError();
}

//---------------------------------------
// Main parseProgram (Tasks 1-5)
//---------------------------------------
void Parser::parseProgram() {
    parseTasksSection();
    parsePolySection();
    parseExecuteSection();
    parseInputsSection();
    Token t = getNextToken();
    if (t.token_type != END_OF_FILE)
        syntaxError();
    checkSemanticErrors();
    doOtherTasks();
}

//---------------------------------------
// TASKS Section
//---------------------------------------
void Parser::parseTasksSection() {
    expect(TASKS);
    vector<int> tasks;
    parseNumList(tasks);
    for (int num : tasks) {
        if (num == 1) doTask1 = true;
        if (num == 2) doTask2 = true;
        if (num == 3) doTask3 = true;
        if (num == 4) doTask4 = true;
        if (num == 5) doTask5 = true;
    }
}

void Parser::parseNumList(vector<int>& numList) {
    Token t = getNextToken();
    if (t.token_type != NUM)
        syntaxError();
    numList.push_back(atoi(t.lexeme.c_str()));
    Token nxt = peekToken();
    if (nxt.token_type == NUM)
        parseNumList(numList);
}

//---------------------------------------
// POLY Section (Tasks 1 & 5)
//---------------------------------------
void Parser::parsePolySection() {
    expect(POLY);
    parsePolyDeclList();
}

void Parser::parsePolyDeclList() {
    Token t = peekToken();
    if (t.token_type == ID) {
        parsePolyDecl();
        t = peekToken();
        if (t.token_type == ID)
            parsePolyDeclList();
        else if (t.token_type == EXECUTE)
            return;
        else
            syntaxError();
    } else {
        syntaxError();
    }
}

void Parser::parsePolyDecl() {
    parsePolyHeader();
    expect(EQUAL);
    int deg = 0;
    ASTNode* root = parseTermListNode(deg);
    polyASTs.push_back(root);
    if (!polyTable.empty())
        polyTable.back().degree = deg;
    expect(SEMICOLON);
}

void Parser::parsePolyHeader() {
    Token polyTok = getNextToken();
    if (polyTok.token_type != ID)
        syntaxError();
    string polyName = polyTok.lexeme;
    int polyLine = polyTok.line_no;
    vector<string> params;
    Token t = peekToken();
    if (t.token_type == LPAREN) {
        expect(LPAREN);
        parseIdList(params);
        expect(RPAREN);
    } else {
        params.push_back("x");
    }
    bool duplicate = false;
    for (auto &ph : polyTable) {
        if (ph.name == polyName) {
            duplicate = true;
            break;
        }
    }
    if (duplicate)
        semErr1Lines.push_back(polyLine);
    else {
        PolyHeader ph;
        ph.name = polyName;
        ph.params = params;
        ph.line = polyLine;
        ph.degree = 0;
        polyTable.push_back(ph);
    }
    currentPolyParams = params;
}

void Parser::parseIdList(vector<string>& params) {
    Token t = getNextToken();
    if (t.token_type != ID)
        syntaxError();
    params.push_back(t.lexeme);
    Token nxt = peekToken();
    if (nxt.token_type == COMMA) {
        expect(COMMA);
        parseIdList(params);
    }
}

void Parser::parsePolyBody() {
    int dummy = 0;
    parseTermListNode(dummy);
}

//---------------------------------------
// AST Building for Polynomial Body (Task 5)
//---------------------------------------
ASTNode* Parser::parseTermListNode(int &outDegree) {
    int dTerm = 0;
    ASTNode* firstTerm = parseTermNode(dTerm);
    ASTNode* termList = new ASTNode();
    termList->kind = NodeKind::TERM_LIST;
    termList->children.push_back(firstTerm);
    outDegree = dTerm;
    Token t = peekToken();
    while (t.token_type == PLUS || t.token_type == MINUS) {
        Token opTok = getNextToken(); // consume operator
        int sign = (opTok.token_type == PLUS) ? +1 : -1;
        int d2 = 0;
        ASTNode* nextTerm = parseTermNode(d2);
        nextTerm->add_op = sign;
        termList->children.push_back(nextTerm);
        if (d2 > outDegree)
            outDegree = d2;
        t = peekToken();
    }
    return termList;
}

ASTNode* Parser::parseTermNode(int &outDegree) {
    Token t = peekToken();
    if (t.token_type == NUM) {
        Token coeffTok = getNextToken();
        int coeff = atoi(coeffTok.lexeme.c_str());
        Token nxt = peekToken();
        if (nxt.token_type == ID || nxt.token_type == LPAREN) {
            int dMono = 0;
            ASTNode* monoList = parseMonomialListNode(dMono);
            outDegree = dMono;
            ASTNode* termNode = new ASTNode();
            termNode->kind = NodeKind::TERM;
            termNode->value = coeff;
            termNode->children.push_back(monoList);
            return termNode;
        } else {
            outDegree = 0;
            ASTNode* termNode = new ASTNode();
            termNode->kind = NodeKind::TERM;
            termNode->value = coeff;
            return termNode;
        }
    } else if (t.token_type == ID || t.token_type == LPAREN) {
        int dMono = 0;
        ASTNode* monoList = parseMonomialListNode(dMono);
        outDegree = dMono;
        ASTNode* termNode = new ASTNode();
        termNode->kind = NodeKind::TERM;
        termNode->value = 1;
        termNode->children.push_back(monoList);
        return termNode;
    } else {
        syntaxError();
        return nullptr;
    }
}

ASTNode* Parser::parseMonomialListNode(int &outDegree) {
    int dMono = 0;
    ASTNode* firstMono = parseMonomialNode(dMono);
    ASTNode* ml = new ASTNode();
    ml->kind = NodeKind::MONO_LIST;
    ml->children.push_back(firstMono);
    outDegree = dMono;
    Token t = peekToken();
    while (t.token_type == ID || t.token_type == LPAREN) {
        int d2 = 0;
        ASTNode* nxt = parseMonomialNode(d2);
        ml->children.push_back(nxt);
        outDegree += d2; // sum degrees of monomials in product
        t = peekToken();
    }
    return ml;
}

ASTNode* Parser::parseMonomialNode(int &outDegree) {
    int dPrim = 0;
    ASTNode* prim = parsePrimaryNode(dPrim);
    int exponent = 1;
    Token t = peekToken();
    if (t.token_type == POWER) {
        exponent = parseExponent();
    }
    outDegree = dPrim * exponent;
    ASTNode* mono = new ASTNode();
    mono->kind = NodeKind::MONO;
    mono->value = exponent;
    mono->children.push_back(prim);
    return mono;
}

ASTNode* Parser::parsePrimaryNode(int &outDegree) {
    Token t = peekToken();
    if (t.token_type == ID) {
        Token idTok = getNextToken();
        bool found = false;
        int idx = -1;
        for (int i = 0; i < (int)currentPolyParams.size(); i++) {
            if (idTok.lexeme == currentPolyParams[i]) {
                found = true;
                idx = i;
                break;
            }
        }
        if (!found)
            semErr2Lines.push_back(idTok.line_no);
        outDegree = 1;
        ASTNode* node = new ASTNode();
        node->kind = NodeKind::PRIMARY;
        node->paramIndex = (found ? idx : -1);
        return node;
    } else if (t.token_type == LPAREN) {
        expect(LPAREN);
        int dTL = 0;
        ASTNode* sub = parseTermListNode(dTL);
        outDegree = dTL;
        expect(RPAREN);
        ASTNode* node = new ASTNode();
        node->kind = NodeKind::PRIMARY;
        node->paramIndex = -1;
        node->children.push_back(sub);
        return node;
    } else {
        syntaxError();
        return nullptr;
    }
}

int Parser::parseExponent() {
    expect(POWER);
    Token t = getNextToken();
    if (t.token_type != NUM)
        syntaxError();
    return atoi(t.lexeme.c_str());
}

//---------------------------------------
// EXECUTE Section (Tasks 2, 3, 4)
//---------------------------------------
void Parser::parseExecuteSection() {
    expect(EXECUTE);
    parseStatementList();
}

void Parser::parseStatementList() {
    Token t = peekToken();
    if (t.token_type == INPUT || t.token_type == OUTPUT || t.token_type == ID) {
        Statement st = parseStatement();
        statements.push_back(st);
        t = peekToken();
        if (t.token_type == INPUT || t.token_type == OUTPUT || t.token_type == ID)
            parseStatementList();
    } else {
        syntaxError();
    }
}

Statement Parser::parseStatement() {
    Statement st;
    st.line = peekToken().line_no;
    Token t = peekToken();
    if (t.token_type == INPUT)
        st = parseInputStatement();
    else if (t.token_type == OUTPUT)
        st = parseOutputStatement();
    else if (t.token_type == ID)
        st = parseAssignStatement();
    else
        syntaxError();
    return st;
}

Statement Parser::parseInputStatement() {
    Statement st;
    st.type = StmtType::INPUT_STMT;
    getNextToken(); // consume INPUT
    Token varTok = getNextToken();
    if (varTok.token_type != ID)
        syntaxError();
    st.varName = varTok.lexeme;
    getLocation(st.varName);
    initializedVars[st.varName] = true;
    expect(SEMICOLON);
    st.line = varTok.line_no;
    return st;
}

Statement Parser::parseOutputStatement() {
    Statement st;
    st.type = StmtType::OUTPUT_STMT;
    getNextToken(); // consume OUTPUT
    Token varTok = getNextToken();
    if (varTok.token_type != ID)
        syntaxError();
    st.varName = varTok.lexeme;
    getLocation(st.varName);
    if (!initializedVars[st.varName])
        uninitWarnLines.push_back(varTok.line_no);
    expect(SEMICOLON);
    st.line = varTok.line_no;
    return st;
}

Statement Parser::parseAssignStatement() {
    Statement st;
    st.type = StmtType::ASSIGN_STMT;
    Token varTok = getNextToken(); // LHS variable
    if (varTok.token_type != ID)
        syntaxError();
    st.lhsVar = varTok.lexeme;
    getLocation(st.lhsVar);
    expect(EQUAL);
    st.rhsEval = parsePolyEvaluationExec();
    expect(SEMICOLON);
    st.line = varTok.line_no;
    initializedVars[st.lhsVar] = true;
    return st;
}

PolyEvalExec Parser::parsePolyEvaluationExec() {
    PolyEvalExec pe;
    Token polyTok = getNextToken();
    if (polyTok.token_type != ID)
        syntaxError();
    int foundIndex = -1;
    for (int i = 0; i < (int)polyTable.size(); i++) {
        if (polyTable[i].name == polyTok.lexeme) {
            foundIndex = i;
            break;
        }
    }
    if (foundIndex < 0)
        semErr3Lines.push_back(polyTok.line_no);
    pe.polyIndex = foundIndex;
    expect(LPAREN);
    vector<PolyEvalArg> args;
    int argCount = parseArgumentListExec(args);
    expect(RPAREN);
    if (foundIndex >= 0) {
        int declared = (int)polyTable[foundIndex].params.size();
        if (argCount != declared)
            semErr4Lines.push_back(polyTok.line_no);
    }
    pe.args = args;
    return pe;
}

int Parser::parseArgumentListExec(vector<PolyEvalArg>& args) {
    int count = 0;
    PolyEvalArg a = parseArgumentExec();
    args.push_back(a);
    count++;
    Token t = peekToken();
    while (t.token_type == COMMA) {
        expect(COMMA);
        PolyEvalArg a2 = parseArgumentExec();
        args.push_back(a2);
        count++;
        t = peekToken();
    }
    return count;
}

PolyEvalArg Parser::parseArgumentExec() {
    PolyEvalArg a;
    Token t = peekToken();
    if (t.token_type == NUM) {
        Token numTok = getNextToken();
        a.kind = ArgKind::NUM;
        a.numValue = atoi(numTok.lexeme.c_str());
    } else if (t.token_type == ID) {
        Token t2 = peekToken(2);
        if (t2.token_type == LPAREN) {
            a.kind = ArgKind::POLY_EVAL;
            a.nested = parsePolyEvaluationExec();
        } else {
            Token varTok = getNextToken();
            a.kind = ArgKind::VAR;
            a.varName = varTok.lexeme;
            getLocation(a.varName);
            if (!initializedVars[a.varName])
                uninitWarnLines.push_back(varTok.line_no);
        }
    } else {
        syntaxError();
    }
    return a;
}

//---------------------------------------
// INPUTS Section (Task 2)
//---------------------------------------
void Parser::parseInputsSection() {
    expect(INPUTS);
    parseNumList(inputValues);
}

//---------------------------------------
// Semantic Error Check (Task 1)
//---------------------------------------
void Parser::checkSemanticErrors() {
    if (!semErr1Lines.empty()) {
        sort(semErr1Lines.begin(), semErr1Lines.end());
        cout << "Semantic Error Code 1:";
        for (int ln : semErr1Lines)
            cout << " " << ln;
        cout << endl;
        exit(0);
    }
    if (!semErr2Lines.empty()) {
        sort(semErr2Lines.begin(), semErr2Lines.end());
        cout << "Semantic Error Code 2:";
        for (int ln : semErr2Lines)
            cout << " " << ln;
        cout << endl;
        exit(0);
    }
    if (!semErr3Lines.empty()) {
        sort(semErr3Lines.begin(), semErr3Lines.end());
        cout << "Semantic Error Code 3:";
        for (int ln : semErr3Lines)
            cout << " " << ln;
        cout << endl;
        exit(0);
    }
    if (!semErr4Lines.empty()) {
        sort(semErr4Lines.begin(), semErr4Lines.end());
        cout << "Semantic Error Code 4:";
        for (int ln : semErr4Lines)
            cout << " " << ln;
        cout << endl;
        exit(0);
    }
}

//---------------------------------------
// Other Tasks Execution (Tasks 2-5)
//---------------------------------------
void Parser::doOtherTasks() {
    if (doTask2)
        executeProgram();
    if (doTask3)
        printUninitializedWarnings();
    if (doTask4) {
        detectUselessAssignments();
        printUselessAssignmentWarnings();
    }
    if (doTask5)
        printPolynomialDegrees();
}

//---------------------------------------
// Task 2: Execution & Evaluation
//---------------------------------------
static const int MEM_SIZE = 2000;
static int memVar[MEM_SIZE];

int Parser::getLocation(const std::string &var) {
    if (varLocation.find(var) == varLocation.end())
        varLocation[var] = nextLoc++;
    return varLocation[var];
}

int Parser::evalPolyEvalExec(const PolyEvalExec &pe) {
    if (pe.polyIndex < 0 || pe.polyIndex >= (int)polyASTs.size())
        return 0;
    std::vector<int> argVals;
    for (auto &A : pe.args) {
        if (A.kind == ArgKind::NUM)
            argVals.push_back(A.numValue);
        else if (A.kind == ArgKind::VAR) {
            int loc = getLocation(A.varName);
            argVals.push_back(memVar[loc]);
        }
        else {
            int nestedVal = evalPolyEvalExec(A.nested);
            argVals.push_back(nestedVal);
        }
    }
    return evalPoly(pe.polyIndex, argVals);
}

void Parser::executeProgram() {
    for (int i = 0; i < MEM_SIZE; i++)
        memVar[i] = 0;
    inputIndex = 0;
    for (auto &st : statements) {
        if (st.type == StmtType::INPUT_STMT) {
            int loc = getLocation(st.varName);
            if (inputIndex < (int)inputValues.size())
                memVar[loc] = inputValues[inputIndex++];
            else
                memVar[loc] = 0;
        } else if (st.type == StmtType::OUTPUT_STMT) {
            int loc = getLocation(st.varName);
            cout << memVar[loc] << endl;
        } else if (st.type == StmtType::ASSIGN_STMT) {
            int lhsLoc = getLocation(st.lhsVar);
            int result = evalPolyEvalExec(st.rhsEval);
            memVar[lhsLoc] = result;
        }
    }
}

int Parser::evalPoly(int polyIndex, const std::vector<int> &args) {
    if (polyIndex < 0 || polyIndex >= (int)polyASTs.size())
        return 0;
    return evalNode(polyASTs[polyIndex], args);
}

int Parser::evalNode(ASTNode* node, const std::vector<int> &args) {
    if (!node)
        return 0;
    switch (node->kind) {
        case NodeKind::TERM_LIST: {
            int total = 0;
            for (auto *ch : node->children) {
                int val = evalNode(ch, args);
                int sign = (ch->add_op == 0) ? 1 : ch->add_op;
                total += sign * val;
            }
            return total;
        }
        case NodeKind::TERM: {
            int coeff = node->value;
            if (!node->children.empty())
                return coeff * evalNode(node->children[0], args);
            else
                return coeff;
        }
        case NodeKind::MONO_LIST: {
            int product = 1;
            for (auto *ch : node->children)
                product *= evalNode(ch, args);
            return product;
        }
        case NodeKind::MONO: {
            int baseVal = evalNode(node->children[0], args);
            int exp = node->value;
            int res = 1;
            for (int i = 0; i < exp; i++)
                res *= baseVal;
            return res;
        }
        case NodeKind::PRIMARY: {
            if (node->paramIndex >= 0) {
                if (node->paramIndex < (int)args.size())
                    return args[node->paramIndex];
                else
                    return 0;
            } else {
                if (!node->children.empty())
                    return evalNode(node->children[0], args);
                return 0;
            }
        }
        default:
            return 0;
    }
}

//---------------------------------------
// Task 3: Uninitialized Variable Warnings
//---------------------------------------
void Parser::printUninitializedWarnings() {
    if (uninitWarnLines.empty())
        return;
    sort(uninitWarnLines.begin(), uninitWarnLines.end());
    cout << "Warning Code 1:";
    for (int ln : uninitWarnLines)
        cout << " " << ln;
    cout << endl;
}

//---------------------------------------
// Task 4: Useless Assignment Warnings
//---------------------------------------
void Parser::collectVarsInPolyEvalExec(const PolyEvalExec &pe, std::unordered_map<std::string, bool> &live) {
    for (auto &A : pe.args) {
        if (A.kind == ArgKind::VAR)
            live[A.varName] = true;
        else if (A.kind == ArgKind::POLY_EVAL)
            collectVarsInPolyEvalExec(A.nested, live);
    }
}

void Parser::detectUselessAssignments() {
    std::unordered_map<std::string, bool> live;
    for (int i = (int)statements.size() - 1; i >= 0; i--) {
        Statement &st = statements[i];
        if (st.type == StmtType::OUTPUT_STMT) {
            live[st.varName] = true;
        } else if (st.type == StmtType::ASSIGN_STMT) {
            if (!live[st.lhsVar])
                uselessWarnLines.push_back(st.line);
            else {
                live.erase(st.lhsVar);
                collectVarsInPolyEvalExec(st.rhsEval, live);
            }
        } else if (st.type == StmtType::INPUT_STMT) {
            if (!live[st.varName])
                uselessWarnLines.push_back(st.line);
            else
                live.erase(st.varName);
        }
    }
}

void Parser::printUselessAssignmentWarnings() {
    if (uselessWarnLines.empty())
        return;
    sort(uselessWarnLines.begin(), uselessWarnLines.end());
    cout << "Warning Code 2:";
    for (int ln : uselessWarnLines)
        cout << " " << ln;
    cout << endl;
}

//---------------------------------------
// Task 5: Print Polynomial Degrees
//---------------------------------------
void Parser::printPolynomialDegrees() {
    for (auto &ph : polyTable) {
        cout << ph.name << ": " << ph.degree << endl;
    }
}

//---------------------------------------
// Main (always included for autograder)
//---------------------------------------
int main() {
    LexicalAnalyzer lexer;
    Parser parser(lexer);
    parser.parseProgram();
    return 0;
}