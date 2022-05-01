#include "Parser.hh"
#include "ByteCode.hh"
#include "object.hh"

// clang-format off
ParseRule Parser::rules_[] = {
    [TokenLeftParen]    = {&Parser::grouping, nullptr, Precedence::None},
    [TokenRightParen]   = {nullptr, nullptr, Precedence::None},
    [TokenLeftBrace]    = {nullptr, nullptr, Precedence::None},
    [TokenRightBrace]   = {nullptr, nullptr, Precedence::None},
    [TokenComma]        = {nullptr, nullptr, Precedence::None},
    [TokenDot]          = {nullptr, nullptr, Precedence::None},
    [TokenMinus]        = {&Parser::unary, &Parser::binary, Precedence::Term},
    [TokenPlus]         = {nullptr, &Parser::binary, Precedence::Term},
    [TokenSemiColon]    = {nullptr, nullptr, Precedence::None},
    [TokenSlash]        = {nullptr, &Parser::binary, Precedence::Factor},
    [TokenStar]         = {nullptr, &Parser::binary, Precedence::Factor},
    [TokenNot]          = {&Parser::unary, nullptr, Precedence::None},
    [TokenNotEqual]     = {nullptr, &Parser::binary, Precedence::Equality},
    [TokenEqual]        = {nullptr, nullptr, Precedence::None},
    [TokenEqualEqual]   = {nullptr, &Parser::binary, Precedence::Equality},
    [TokenGreater]      = {nullptr, &Parser::binary, Precedence::Comparison},
    [TokenGreaterEqual] = {nullptr, &Parser::binary, Precedence::Comparison},
    [TokenLess]         = {nullptr, &Parser::binary, Precedence::Comparison},
    [TokenLessEqual]    = {nullptr, &Parser::binary, Precedence::Comparison},
    [TokenIdentifier]   = {nullptr, nullptr, Precedence::None},
    [TokenString]       = {&Parser::string, nullptr, Precedence::None},
    [TokenNumber]       = {&Parser::number, nullptr, Precedence::None},
    [TokenAnd]          = {nullptr, nullptr, Precedence::None},
    [TokenClass]        = {nullptr, nullptr, Precedence::None},
    [TokenElse]         = {nullptr, nullptr, Precedence::None},
    [TokenFalse]        = {&Parser::literal, nullptr, Precedence::None},
    [TokenFor]          = {nullptr, nullptr, Precedence::None},
    [TokenFun]          = {nullptr, nullptr, Precedence::None},
    [TokenIf]           = {nullptr, nullptr, Precedence::None},
    [TokenNil]          = {&Parser::literal, nullptr, Precedence::None},
    [TokenOr]           = {nullptr, nullptr, Precedence::None},
    [TokenPrint]        = {nullptr, nullptr, Precedence::None},
    [TokenReturn]       = {nullptr, nullptr, Precedence::None},
    [TokenParent]       = {nullptr, nullptr, Precedence::None},
    [TokenThis]         = {nullptr, nullptr, Precedence::None},
    [TokenTrue]         = {&Parser::literal, nullptr, Precedence::None},
    [TokenDef]          = {nullptr, nullptr, Precedence::None},
    [TokenWhile]        = {nullptr, nullptr, Precedence::None},
    [TokenError]        = {nullptr, nullptr, Precedence::None},
    [TokenEof]          = {nullptr, nullptr, Precedence::None},
};

// clang-format on

ParseRule* Parser::getRule(TokenType type) {
    return &rules_[type];
}

Parser::Parser(Scanner& scanner, ByteCode& byteCode) : scanner_{scanner},
                                                       byteCode_{byteCode} {}

void Parser::advance() {
    previous_ = current_;

    while (true) {
        current_ = scanner_.scanToken();
        if (current_.type != TokenError) break;

        errorAtCurrent(current_.name);
    }
}

void Parser::consume(TokenType type, std::string_view msg) {
    if (current_.type == type) {
        advance();
        return;
    }

    errorAtCurrent(msg);
}

void Parser::errorAtCurrent(std::string_view msg) {
    errorAt(current_, msg);
}

void Parser::error(std::string_view msg) {
    errorAt(previous_, msg);
}

void Parser::errorAt(Token& token, std::string_view msg) {
    if (panicMode_) return;

    panicMode_ = true;

    fmt::print("[line {}] Error", token.line);

    if (token.type == TokenEof) {
        fmt::print(" at end");
    } else if (token.type == TokenError) {
        // Nothing
    } else {
        fmt::print(" at '{}'", token.name);
    }

    fmt::print(": {}\n", msg);
    hadError_ = true;
    DEBUG_BREAK();
}

void Parser::emitByte(u8 byte) {
    byteCode_.writeByte(byte, previous_.line);
}

void Parser::emitOpCode(OpCode code) {
    byteCode_.writeOpCode(code, previous_.line);
}

void Parser::emitOpCodes(OpCode code1, OpCode code2) {
    emitOpCode(code1);
    emitOpCode(code2);
}
void Parser::emitReturn() {
    emitOpCode(OpCode::Return);
}

void Parser::emitOpCodeAndOperand(OpCode code, u8 operand) {
    emitOpCode(code);
    emitByte(operand);
}

void Parser::emitConstant(Value value) {
    // byteCode_.writeConstantInstr(value, previous_.line);
    auto operand = byteCode_.writeValue(value);

    if (operand > UINT8_MAX) {
        error("Too many constants in one chunk");
        return;
    }

    emitOpCodeAndOperand(OpCode::Constant, static_cast<u8>(operand));
}

void Parser::expression() {
    parsePrecedence(Precedence::Assignment);
}

void Parser::number() {
    double value = strtod(previous_.name.data(), nullptr);
    emitConstant(Value::createNumber(value));
}

void Parser::grouping() {
    expression();
    consume(TokenRightParen, "Expect ')' after expression");
}
void Parser::unary() {
    auto operatorType = previous_.type;

    parsePrecedence(Precedence::Unary);

    switch (operatorType) {
    case TokenMinus:
        emitOpCode(OpCode::Negate);
        break;
    case TokenNot:
        emitOpCode(OpCode::Not);
        break;
    default:
        return;
    }
}

void Parser::literal() {
    switch (previous_.type) {
    case TokenTrue:
        emitOpCode(OpCode::True);
        break;
    case TokenFalse:
        emitOpCode(OpCode::False);
        break;
    case TokenNil:
        emitOpCode(OpCode::Nil);
        break;
    default:
        return;
    }
}

void Parser::parsePrecedence(Precedence precedence) {
    advance();
    auto prefixRule = getRule(previous_.type)->prefix;
    if (prefixRule == nullptr) {
        error("Expect expression");
        return;
    }

    prefixRule(this);

    while (precedence <= getRule(current_.type)->precedence) {
        advance();
        auto infixRule = getRule(previous_.type)->infix;
        infixRule(this);
    }
}

void Parser::binary() {
    auto operatorType = previous_.type;

    auto rule = getRule(operatorType);
    parsePrecedence((Precedence)(rule->precedence + 1));

    switch (operatorType) {
    case TokenPlus:
        emitOpCode(OpCode::Add);
        break;
    case TokenMinus:
        emitOpCode(OpCode::Subtract);
        break;
    case TokenStar:
        emitOpCode(OpCode::Multiply);
        break;
    case TokenSlash:
        emitOpCode(OpCode::Divide);
        break;
    case TokenNotEqual:
        emitOpCodes(OpCode::Equal, OpCode::Not);
        break;
    case TokenEqualEqual:
        emitOpCode(OpCode::Equal);
        break;
    case TokenGreater:
        emitOpCode(OpCode::Greater);
        break;
    case TokenLess:
        emitOpCode(OpCode::Less);
        break;
    case TokenGreaterEqual:
        emitOpCodes(OpCode::Less, OpCode::Not);
        break;
    case TokenLessEqual:
        emitOpCodes(OpCode::Greater, OpCode::Not);
        break;

    default:
        return;
    }
}

void Parser::string() {
    emitConstant(Value::createObj(ObjFactory::copyString(previous_.name.data() + 1, previous_.name.length() - 2)));
}
