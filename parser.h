#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <unordered_map>
#include "lexer.h"

///---------------------------------------------------------
/// Data Structures for Polynomial Declarations (Tasks 1 & 5)
///---------------------------------------------------------

/// Represents a polynomial declaration, storing its name, parameters, 
/// the line number where it was first declared, and its computed degree.
struct PolyHeader {
    std::string name;
    std::vector<std::string> params; // Parameter names
    int line;                        // First declaration line
    int degree;                      // Computed polynomial degree
};

///---------------------------------------------------------
/// Abstract Syntax Tree (AST) for Polynomial Body (Task 5)
///---------------------------------------------------------

/// Enumeration for different node types in the AST.
enum class NodeKind { TERM_LIST, TERM, MONO_LIST, MONO, PRIMARY, NONE };

/// Represents a node in the AST for polynomial expressions.
struct ASTNode {
    NodeKind kind;                  // Type of AST node
    int value;                       // Coefficient or exponent value
    int add_op;                      // +1 for plus, -1 for minus, 0 if none
    int paramIndex;                  // Index of parameter if PRIMARY node; -1 otherwise
    std::vector<ASTNode*> children;  // Child nodes

    ASTNode() : kind(NodeKind::NONE), value(0), add_op(0), paramIndex(-1) {}
};

///---------------------------------------------------------
/// Data Structures for Program Execution (Task 2)
///---------------------------------------------------------

/// Enumeration of different statement types in the EXECUTE section.
enum class StmtType { INPUT_STMT, OUTPUT_STMT, ASSIGN_STMT };

/// Forward declaration for polynomial evaluation structure.
struct PolyEvalArg;

/// Represents the evaluation of a polynomial during execution.
struct PolyEvalExec {
    int polyIndex;                 // Index in polyTable (-1 if not found)
    std::vector<PolyEvalArg> args; // Arguments for evaluation
};

/// Enumeration for different kinds of evaluation arguments.
enum class ArgKind { VAR, NUM, POLY_EVAL };

/// Represents an argument in a polynomial evaluation.
struct PolyEvalArg {
    ArgKind kind;               // Type of argument
    std::string varName;        // Variable name (if kind == VAR)
    int numValue;               // Numeric value (if kind == NUM)
    PolyEvalExec nested;        // Nested polynomial evaluation (if kind == POLY_EVAL)

    PolyEvalArg() : kind(ArgKind::VAR), numValue(0) {}
};

/// Represents a single statement in the program (input, output, or assignment).
struct Statement {
    StmtType type;            // Type of statement
    int line;                 // Line number of the statement
    std::string varName;      // Used for INPUT/OUTPUT statements
    std::string lhsVar;       // Left-hand side variable in ASSIGN statements
    PolyEvalExec rhsEval;     // Right-hand side evaluation in ASSIGN statements
};

///---------------------------------------------------------
/// Parser Class Declaration
///---------------------------------------------------------

/// Parses the input program, handling polynomial declarations,
/// execution statements, and error detection.
class Parser {
public:
    Parser(LexicalAnalyzer &lexer);
    void parseProgram();

private:
    LexicalAnalyzer &lexer;

    /// Task flags (set in TASKS section)
    bool doTask1 = false; // Always performed internally
    bool doTask2 = false;
    bool doTask3 = false;
    bool doTask4 = false;
    bool doTask5 = false;

    /// Semantic error tracking (Task 1)
    std::vector<int> semErr1Lines; // Duplicate polynomial declarations
    std::vector<int> semErr2Lines; // Invalid monomial names
    std::vector<int> semErr3Lines; // Undeclared polynomial evaluations
    std::vector<int> semErr4Lines; // Incorrect number of arguments

    /// Storage for polynomial declarations and their AST representations (Tasks 1 & 5)
    std::vector<PolyHeader> polyTable;    // List of declared polynomials
    std::vector<ASTNode*> polyASTs;       // ASTs for polynomial bodies
    std::vector<std::string> currentPolyParams; // Active parameters while parsing a polynomial

    /// Statement list for execution (Tasks 2, 3, 4)
    std::vector<Statement> statements;

    /// Input value storage (Task 2)
    std::vector<int> inputValues;
    int inputIndex = 0;

    /// Uninitialized variable tracking (Task 3)
    std::vector<int> uninitWarnLines;
    std::unordered_map<std::string, bool> initializedVars;

    /// Variable memory allocation (Task 2)
    std::unordered_map<std::string, int> varLocation;
    int nextLoc = 0;
    int getLocation(const std::string &var);

    /// Warnings for useless assignments (Task 4)
    std::vector<int> uselessWarnLines;

    /// Helper functions for token handling
    Token getNextToken();
    Token peekToken(int k = 1);
    void expect(TokenType expected);
    void syntaxError();

    /// Parsing routines for program structure
    void parseTasksSection();
    void parseNumList(std::vector<int>& numList);
    void parsePolySection();
    void parsePolyDeclList();
    void parsePolyDecl();
    void parsePolyHeader();
    void parseIdList(std::vector<std::string>& params);
    void parsePolyBody();

    /// AST construction for polynomial expressions (Task 5)
    ASTNode* parseTermListNode(int &outDegree);
    ASTNode* parseTermNode(int &outDegree);
    ASTNode* parseMonomialListNode(int &outDegree);
    ASTNode* parseMonomialNode(int &outDegree);
    ASTNode* parsePrimaryNode(int &outDegree);
    int parseExponent();

    /// Parsing routines for EXECUTE section (Tasks 2, 3, 4)
    void parseExecuteSection();
    void parseStatementList();
    Statement parseStatement();
    Statement parseInputStatement();
    Statement parseOutputStatement();
    Statement parseAssignStatement();
    PolyEvalExec parsePolyEvaluationExec();
    int parseArgumentListExec(std::vector<PolyEvalArg>& args);
    PolyEvalArg parseArgumentExec();

    void parseInputsSection();

    /// Semantic error detection (Task 1)
    void checkSemanticErrors();

    /// Execution tasks (Tasks 2-5)
    void doOtherTasks();
    void executeProgram();
    int evalPoly(int polyIndex, const std::vector<int>& args);
    int evalNode(ASTNode* node, const std::vector<int>& args);
    int evalPolyEvalExec(const PolyEvalExec &pe);
    void printUninitializedWarnings();
    void detectUselessAssignments();
    void collectVarsInPolyEvalExec(const PolyEvalExec &pe, std::unordered_map<std::string, bool> &live);
    void printUselessAssignmentWarnings();
    void printPolynomialDegrees();
};

#endif
