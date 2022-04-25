#pragma once

#include "common.hh"
#include "Scanner.hh"
#include "instructions.hh"
#include "Value.hh"

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

using ParseFn = std::function<void(Parser*)>;

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

    // for code generation
    void emitByte(u8 byte);
    void emitReturn();
    void emitOpCode(OpCode code);
    void emitOpCodeAndOperand(OpCode code, u8 operand);
    void emitConstant(Value value);
    void emitOpCodes(OpCode code1, OpCode code2);

private:
    void errorAtCurrent(std::string_view msg);
    void error(std::string_view msg);
    void errorAt(Token& token, std::string_view msg);

public:
    void expression();
    void number();
    void grouping();
    void unary();
    void parsePrecedence(Precedence precedence);
    void binary();
    void literal();
    void string();

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
