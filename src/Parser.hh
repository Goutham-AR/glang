#pragma once

#include "common.hh"
#include "Scanner.hh"
#include "instructions.hh"
#include "Value.hh"
#include "compiler.hh"

#include <string_view>
#include <functional>

class Parser;

enum class Precedence {
    None,
    Assignment,
    Or,
    And,
    Equality,
    Comparison,
    Term,
    Factor,
    Unary,
    Call,
    Primary
};

inline Precedence operator+(Precedence p, int i) {
    auto a = static_cast<int>(p);
    a = a + i;
    return static_cast<Precedence>(a);
}

using ParseFn = std::function<void(Parser*, bool)>;

struct ParseRule {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
};

class ByteCode;

class Parser {
public:
    explicit Parser(Scanner& scanner, ByteCode& byteCode);
    ~Parser() = default;

    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;

    void advance();
    void consume(TokenType type, std::string_view msg);
    [[nodiscard]] bool hasError() const { return hadError_; }
    bool match(TokenType type);

    // for code generation
    void emitByte(u8 byte);
    void emitReturn();
    void emitOpCode(OpCode code);
    void emitOpCodeAndOperand(OpCode code, u8 operand);
    void emitConstant(Value value);
    void emitOpCodes(OpCode code1, OpCode code2);
    int emitJump(OpCode opcode);
    void emitLoop(int loopstart);
    void patchJump(int offset);
    void and_(bool canAssign);
    void or_(bool canAssign);

private:
    void errorAtCurrent(std::string_view msg);
    void error(std::string_view msg);
    void errorAt(Token& token, std::string_view msg);
    [[nodiscard]] bool check(TokenType type) const {
        return current_.type == type;
    }
    void synchronize();

    u8 parseVariable(std::string_view errorMsg);
    void defineVariable(u8 global);
    u8 identifierConstant(Token* name);

    void namedVariable(Token name, bool canAssign);
    void beginScope();
    void block();
    void endScope();
    void declareVariable();
    void addLocal(Token name);
    int resolveLocal(Compiler* compiler, Token* name);
    void markInitialized();

public:
    void ifStatement();
    void declaration();
    void variableDeclaration();
    void statement();
    void printStatement();
    void forStatement();
    void expressionStatement();
    void whileStatement();
    void expression();
    void number(bool canAssign);
    void grouping(bool canAssign);
    void unary(bool canAssign);
    void parsePrecedence(Precedence precedence);
    void binary(bool canAssign);
    void literal(bool canAssign);
    void string(bool canAssign);
    void variable(bool canAssign);

private:
    Scanner& scanner_;
    ByteCode& byteCode_;
    Token current_{};
    Token previous_{};
    bool hadError_ = false;
    bool panicMode_ = false;

private:
    static ParseRule rules_[];
    static ParseRule* getRule(TokenType type);
};
