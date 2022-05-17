#include "Parser.hh"
#include "ByteCode.hh"
#include "object.hh"
#include "compiler.hh"

static bool identifiersEqual(Token* a, Token* b) {
    if (a->name.length() != b->name.length()) return false;
    return a->name == b->name;
}

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
    [TokenIdentifier]   = {&Parser::variable, nullptr, Precedence::None},
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

void Parser::number(bool canAssign) {
    double value = strtod(previous_.name.data(), nullptr);
    emitConstant(Value::createNumber(value));
}

void Parser::grouping(bool canAssign) {
    expression();
    consume(TokenRightParen, "Expect ')' after expression");
}
void Parser::unary(bool canAssign) {
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

void Parser::literal(bool canAssign) {
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

    bool canAssign = (precedence <= Precedence::Assignment);
    prefixRule(this, canAssign);

    while (precedence <= getRule(current_.type)->precedence) {
        advance();
        auto infixRule = getRule(previous_.type)->infix;
        infixRule(this, canAssign);
    }

    if (canAssign && match(TokenEqual)) {
        error("Invalid assignment target.");
    }
}

void Parser::binary(bool canAssign) {
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

void Parser::string(bool canAssign) {
    emitConstant(Value::createObj(ObjFactory::copyString(previous_.name.data() + 1, previous_.name.length() - 2)));
}

void Parser::variable(bool canAssigns) {
    namedVariable(previous_, canAssigns);
}
int Parser::resolveLocal(Compiler* compiler, Token* name) {
    for (int i = compiler->localCount - 1; i >= 0; i--) {
        Local* local = &compiler->locals[i];
        if (identifiersEqual(name, &local->name)) {
            if (local->depth == -1) {
                error("Can't read local variable in its own initializer.");
            }

            return i;
        }
    }
    return -1;
}

void Parser::namedVariable(Token name, bool canAssign) {

    OpCode getOp, setOp;
    int arg = resolveLocal(g_current, &name);

    if (arg != -1) {
        getOp = OpCode::GetLocal;
        setOp = OpCode::SetLocal;
    } else {
        arg = identifierConstant(&name);
        getOp = OpCode::GetGlobal;
        setOp = OpCode::SetGlobal;
    }

    if (canAssign && match(TokenEqual)) {
        expression();
        emitOpCodeAndOperand(setOp, arg);
    } else {
        emitOpCodeAndOperand(getOp, arg);
    }
}

bool Parser::match(TokenType type) {
    if (!check(type)) return false;
    advance();
    return true;
}

void Parser::declaration() {
    if (match(TokenDef)) {
        variableDeclaration();
    } else {
        statement();
    }

    if (panicMode_) {
        synchronize();
    }
}

void Parser::variableDeclaration() {
    u8 global = parseVariable("Expect variable name.");

    if (match(TokenEqual)) {
        expression();
    } else {
        emitOpCode(OpCode::Nil);
    }

    consume(TokenSemiColon, "Expect ';' after variable declaration");
    defineVariable(global);
}

void Parser::statement() {
    if (match(TokenPrint)) {
        printStatement();
    } else if (match(TokenLeftBrace)) {
        beginScope();
        block();
        endScope();
    } else {
        expressionStatement();
    }
}

void Parser::beginScope() {
    g_current->scopeDepth++;
}
void Parser::block() {
    while (!check(TokenRightBrace) && !check(TokenEof)) {
        declaration();
    }

    consume(TokenRightBrace, "Expect '}' after block");
}
void Parser::endScope() {
    g_current->scopeDepth--;

    while (g_current->localCount > 0 && g_current->locals[g_current->localCount - 1].depth > g_current->scopeDepth) {
        emitOpCode(OpCode::Pop);
        g_current->localCount--;
    }
}

void Parser::printStatement() {
    expression();
    consume(TokenSemiColon, "Expect ';' after value.");
    emitOpCode(OpCode::Print);
}

void Parser::expressionStatement() {
    expression();
    consume(TokenSemiColon, "Expect ';' after expression");
    emitOpCode(OpCode::Pop);
}

/*
We skip tokens until we reach something that looks like a statement boundary.
We recognize the boundary by looking for a preceding token that can end a statement, like a semicolon.
Or we’ll look for a subsequent token that begins a statement, usually one of the control flow or declaration keywords.
*/
void Parser::synchronize() {
    panicMode_ = false;

    while (current_.type != TokenEof) {
        if (previous_.type == TokenSemiColon) return;

        switch (current_.type) {
        case TokenClass:
        case TokenFun:
        case TokenDef:
        case TokenFor:
        case TokenIf:
        case TokenWhile:
        case TokenPrint:
        case TokenReturn:
            return;
        default:;
        }
        advance();
    }
}

u8 Parser::parseVariable(std::string_view errorMsg) {
    consume(TokenIdentifier, errorMsg);
    declareVariable();
    if (g_current->scopeDepth > 0) return 0;

    return identifierConstant(&previous_);
}

void Parser::declareVariable() {
    if (g_current->scopeDepth == 0) return;

    Token* name = &previous_;
    for (int i = g_current->localCount - 1; i >= 0; i--) {
        Local* local = &g_current->locals[i];
        if (local->depth != -1 && local->depth < g_current->scopeDepth) {
            break;
        }

        if (identifiersEqual(name, &local->name)) {
            error("Already variable with this name in this scope");
        }
    }

    addLocal(*name);
}

void Parser::addLocal(Token name) {
    if (g_current->localCount == UINT8_MAX + 1) {
        error("Too many local variables in function");
        return;
    }

    Local* local = &g_current->locals[g_current->localCount++];
    local->name = name;
    local->depth = -1;
}

void Parser::markInitialized() {
    g_current->locals[g_current->localCount - 1].depth = g_current->scopeDepth;
}

void Parser::defineVariable(u8 global) {
    if (g_current->scopeDepth > 0) {
        markInitialized();
        return;
    }

    emitOpCodeAndOperand(OpCode::DefineGlobal, global);
}

u8 Parser::identifierConstant(Token* name) {
    return byteCode_.writeValue(Value::createObj(ObjFactory::copyString(name->name.data(), name->name.length())));
}
