/*
 * File: Basic.cpp
 * ---------------
 * This file is the starter project for the BASIC interpreter.
 */

#include <cctype>
#include <iostream>
#include <string>
#include "exp.hpp"
#include "parser.hpp"
#include "program.hpp"
#include "Utils/error.hpp"
#include "Utils/tokenScanner.hpp"
#include "Utils/strlib.hpp"


/* Function prototypes */

void processLine(std::string line, Program &program, EvalState &state);

/* Main program */

int main() {
    EvalState state;
    Program program;
    //cout << "Stub implementation of BASIC" << endl;
    while (true) {
        try {
            std::string input;
            getline(std::cin, input);
            if (input.empty())
                continue;
            processLine(input, program, state);
        } catch (ErrorException &ex) {
            std::cout << ex.getMessage() << std::endl;
        }
    }
    return 0;
}

/*
 * Function: processLine
 * Usage: processLine(line, program, state);
 * -----------------------------------------
 * Processes a single line entered by the user.  In this version of
 * implementation, the program reads a line, parses it as an expression,
 * and then prints the result.  In your implementation, you will
 * need to replace this method with one that can respond correctly
 * when the user enters a program line (which begins with a number)
 * or one of the BASIC commands, such as LIST or RUN.
 */

void processLine(std::string line, Program &program, EvalState &state) {
    TokenScanner scanner;
    scanner.ignoreWhitespace();
    scanner.scanNumbers();
    scanner.setInput(line);
    // Distinguish numbered lines vs immediate commands
    if (!scanner.hasMoreTokens()) return;
    std::string first = scanner.nextToken();
    TokenType ttype = scanner.getTokenType(first);
    if (ttype == NUMBER) {
        int lineNumber = stringToInteger(first);
        // If nothing follows, remove line
        if (!scanner.hasMoreTokens()) {
            program.removeSourceLine(lineNumber);
            return;
        }
        std::string rest = line.substr(line.find(first) + first.size());
        // Store full original line (including number and space + rest)
        program.addSourceLine(lineNumber, line);
        // Parse statement after the line number
        TokenScanner sc2;
        sc2.ignoreWhitespace();
        sc2.scanNumbers();
        sc2.setInput(trim(rest));
        std::string keyword = toUpperCase(sc2.nextToken());
        Statement *stmt = nullptr;
        if (keyword == "REM") {
            stmt = new RemStatement(sc2);
        } else if (keyword == "LET") {
            stmt = new LetStatement(sc2);
        } else if (keyword == "PRINT") {
            stmt = new PrintStatement(sc2);
        } else if (keyword == "INPUT") {
            stmt = new InputStatement(sc2);
        } else if (keyword == "END") {
            stmt = new EndStatement();
        } else if (keyword == "GOTO") {
            stmt = new GotoStatement(sc2);
        } else if (keyword == "IF") {
            stmt = new IfStatement(sc2);
        } else {
            error("SYNTAX ERROR");
        }
        program.setParsedStatement(lineNumber, stmt);
        return;
    }

    // Immediate commands
    std::string cmd = toUpperCase(first);
    if (cmd == "REM") {
        // ignore rest
        return;
    } else if (cmd == "LET") {
        Statement *stmt = new LetStatement(scanner);
        stmt->execute(state, program);
        delete stmt;
        return;
    } else if (cmd == "PRINT") {
        Statement *stmt = new PrintStatement(scanner);
        stmt->execute(state, program);
        delete stmt;
        return;
    } else if (cmd == "INPUT") {
        Statement *stmt = new InputStatement(scanner);
        stmt->execute(state, program);
        delete stmt;
        return;
    } else if (cmd == "END") {
        // In immediate mode END does nothing visible
        return;
    } else if (cmd == "GOTO") {
        Statement *stmt = new GotoStatement(scanner);
        // In immediate mode, jump has no meaning; ignore
        delete stmt;
        return;
    } else if (cmd == "IF") {
        Statement *stmt = new IfStatement(scanner);
        // Immediate IF-THEN does nothing visible unless combined with GOTO semantics; ignore
        delete stmt;
        return;
    } else if (cmd == "LIST") {
        // Print stored program in order, original lines
        int ln = program.getFirstLineNumber();
        while (ln != -1) {
            std::string src = program.getSourceLine(ln);
            std::cout << src << std::endl;
            ln = program.getNextLineNumber(ln);
        }
        return;
    } else if (cmd == "CLEAR") {
        program.clear();
        state.Clear();
        return;
    } else if (cmd == "QUIT") {
        exit(0);
    } else if (cmd == "RUN") {
        // Execute program from first line following control flow
        program.resetRuntime();
        int pc = program.getFirstLineNumber();
        while (pc != -1) {
            Statement *stmt = program.getParsedStatement(pc);
            if (stmt != nullptr) {
                stmt->execute(state, program);
            }
            if (program.isStopped()) break;
            if (program.hasJump()) {
                int target = program.consumeJump();
                // jump must target an existing line, otherwise error
                if (program.getSourceLine(target) == "") {
                    std::cout << "LINE NUMBER ERROR" << std::endl;
                    break;
                }
                int next = target;
                pc = next;
            } else {
                pc = program.getNextLineNumber(pc);
            }
        }
        return;
    } else {
        error("SYNTAX ERROR");
    }
}
