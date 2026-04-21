/*
 * File: statement.cpp
 * -------------------
 * This file implements the constructor and destructor for
 * the Statement class itself.  Your implementation must do
 * the same for the subclasses you define for each of the
 * BASIC statements.
 */

#include "statement.hpp"
#include "program.hpp"


/* Implementation of the Statement class */

int stringToInt(std::string str);

Statement::Statement() = default;

Statement::~Statement() = default;

// REM: do nothing
RemStatement::RemStatement(TokenScanner &scanner) {
    // consume the rest of the line as comment tokens
    while (scanner.hasMoreTokens()) scanner.nextToken();
}

void RemStatement::execute(EvalState &state, Program &program) {
    (void) state;
    (void) program;
}

// LET: parse expression with assignment
LetStatement::LetStatement(TokenScanner &scanner) {
    // For LET, we expect an assignment expression like x = <exp>
    // We let parser handle it by building a CompoundExp("=", IdentifierExp, RHS)
    exp = parseExp(scanner);
}

LetStatement::~LetStatement() {
    delete exp;
}

void LetStatement::execute(EvalState &state, Program &program) {
    (void) program;
    // Evaluate assignment expression
    exp->eval(state);
}

// PRINT
PrintStatement::PrintStatement(TokenScanner &scanner) {
    exp = parseExp(scanner);
}

PrintStatement::~PrintStatement() {
    delete exp;
}

void PrintStatement::execute(EvalState &state, Program &program) {
    (void) program;
    int v = exp->eval(state);
    std::cout << v << std::endl;
}

// INPUT
InputStatement::InputStatement(TokenScanner &scanner) {
    std::string token = scanner.nextToken();
    if (token.empty()) error("SYNTAX ERROR");
    TokenType type = scanner.getTokenType(token);
    if (type != WORD) error("SYNTAX ERROR");
    var = token;
}

void InputStatement::execute(EvalState &state, Program &program) {
    (void) program;
    std::string line;
    while (true) {
        std::cout << " ? ";
        if (!std::getline(std::cin, line)) {
            // EOF, treat as 0
            state.setValue(var, 0);
            break;
        }
        // accept optional leading + or - followed by digits only
        std::string s = trim(line);
        if (s.empty()) { std::cout << "INVALID NUMBER" << std::endl; continue; }
        size_t pos = 0;
        if (s[0] == '+' || s[0] == '-') {
            if (s.size() == 1) { std::cout << "INVALID NUMBER" << std::endl; continue; }
            pos = 1;
        }
        bool ok = true;
        for (size_t i = pos; i < s.size(); ++i) {
            if (!isdigit(static_cast<unsigned char>(s[i]))) { ok = false; break; }
        }
        if (!ok) { std::cout << "INVALID NUMBER" << std::endl; continue; }
        int val = 0;
        try {
            val = std::stoi(s);
        } catch (...) {
            std::cout << "INVALID NUMBER" << std::endl; continue;
        }
        state.setValue(var, val);
        break;
    }
}

// END
void EndStatement::execute(EvalState &state, Program &program) {
    (void) state;
    program.stop();
}

// GOTO
GotoStatement::GotoStatement(TokenScanner &scanner) {
    std::string tok = scanner.nextToken();
    if (tok.empty() || scanner.getTokenType(tok) != NUMBER) error("SYNTAX ERROR");
    target = stringToInteger(tok);
}

void GotoStatement::execute(EvalState &state, Program &program) {
    (void) state;
    program.jumpTo(target);
}

// IF ... THEN line
IfStatement::IfStatement(TokenScanner &scanner) {
    // parse lhs, op, rhs, THEN, line
    lhs = readE(scanner, 1);
    std::string token = scanner.nextToken();
    if (token != "=" && token != "<" && token != ">") error("SYNTAX ERROR");
    op = token;
    rhs = readE(scanner, 1);
    std::string thenTok = scanner.nextToken();
    if (toUpperCase(thenTok) != "THEN") error("SYNTAX ERROR");
    std::string lineTok = scanner.nextToken();
    if (lineTok.empty() || scanner.getTokenType(lineTok) != NUMBER) error("SYNTAX ERROR");
    target = stringToInteger(lineTok);
}

IfStatement::~IfStatement() {
    delete lhs;
    delete rhs;
}

void IfStatement::execute(EvalState &state, Program &program) {
    int a = lhs->eval(state);
    int b = rhs->eval(state);
    bool cond = false;
    if (op == "=") cond = (a == b);
    else if (op == "<") cond = (a < b);
    else if (op == ">") cond = (a > b);
    if (cond) program.jumpTo(target);
}
