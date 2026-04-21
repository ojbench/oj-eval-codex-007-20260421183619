/*
 * File: program.cpp
 * -----------------
 * This file is a stub implementation of the program.h interface
 * in which none of the methods do anything beyond returning a
 * value of the correct type.  Your job is to fill in the bodies
 * of each of these methods with an implementation that satisfies
 * the performance guarantees specified in the assignment.
 */

#include "program.hpp"
using std::string;



Program::Program() = default;

Program::~Program() {
    clear();
}

void Program::clear() {
    // free any parsed statements
    for (auto &kv : statements) {
        delete kv.second;
    }
    statements.clear();
    sourceLines.clear();
    order.clear();
    jumpLine = -1;
    stopFlag = false;
}

void Program::addSourceLine(int lineNumber, const std::string &line) {
    // Replace or add source line; delete any previous parsed statement
    auto it = sourceLines.find(lineNumber);
    if (it != sourceLines.end()) {
        // replacing existing
        auto sit = statements.find(lineNumber);
        if (sit != statements.end()) {
            delete sit->second;
            statements.erase(sit);
        }
        it->second = line;
    } else {
        sourceLines.emplace(lineNumber, line);
        order.insert(lineNumber);
    }
}

void Program::removeSourceLine(int lineNumber) {
    auto it = sourceLines.find(lineNumber);
    if (it != sourceLines.end()) {
        sourceLines.erase(it);
        order.erase(lineNumber);
        auto sit = statements.find(lineNumber);
        if (sit != statements.end()) {
            delete sit->second;
            statements.erase(sit);
        }
    }
}

std::string Program::getSourceLine(int lineNumber) {
    auto it = sourceLines.find(lineNumber);
    if (it == sourceLines.end()) return "";
    return it->second;
}

void Program::setParsedStatement(int lineNumber, Statement *stmt) {
    auto it = sourceLines.find(lineNumber);
    if (it == sourceLines.end()) {
        error("LINE NUMBER ERROR");
    }
    auto sit = statements.find(lineNumber);
    if (sit != statements.end()) {
        delete sit->second;
        sit->second = stmt;
    } else {
        statements.emplace(lineNumber, stmt);
    }
}

//void Program::removeSourceLine(int lineNumber) {

Statement *Program::getParsedStatement(int lineNumber) {
   auto it = statements.find(lineNumber);
   if (it == statements.end()) return nullptr;
   return it->second;
}

int Program::getFirstLineNumber() {
    if (order.empty()) return -1;
    return *order.begin();
}

int Program::getNextLineNumber(int lineNumber) {
    auto it = order.upper_bound(lineNumber);
    if (it == order.end()) return -1;
    return *it;
}

//more func to add
//todo

void Program::jumpTo(int lineNumber) {
    jumpLine = lineNumber;
}

bool Program::hasJump() const {
    return jumpLine != -1;
}

int Program::consumeJump() {
    int v = jumpLine;
    jumpLine = -1;
    return v;
}

void Program::stop() {
    stopFlag = true;
}

bool Program::isStopped() const {
    return stopFlag;
}

void Program::resetRuntime() {
    jumpLine = -1;
    stopFlag = false;
}
