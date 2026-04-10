#include "parser.h"
#include "object.h"
#include "compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Forward declarations for parse functions referenced in the rules table ── */
static void parse_number(Parser* p, bool can_assign);
static void parse_grouping(Parser* p, bool can_assign);
static void parse_unary(Parser* p, bool can_assign);
static void parse_binary(Parser* p, bool can_assign);
static void parse_literal(Parser* p, bool can_assign);
static void parse_string(Parser* p, bool can_assign);
static void parse_variable(Parser* p, bool can_assign);
static void parse_and(Parser* p, bool can_assign);
static void parse_or(Parser* p, bool can_assign);

/* ── Forward declarations for statements called before their definition ── */
static void variable_declaration(Parser* p);
static void expression_statement(Parser* p);

/* ── Pratt parser rules table (indexed by TokenType) ── */
static ParseRule rules[] = {
    [TokenLeftParen]    = {parse_grouping, NULL,         PREC_NONE},
    [TokenRightParen]   = {NULL,           NULL,         PREC_NONE},
    [TokenLeftBrace]    = {NULL,           NULL,         PREC_NONE},
    [TokenRightBrace]   = {NULL,           NULL,         PREC_NONE},
    [TokenComma]        = {NULL,           NULL,         PREC_NONE},
    [TokenDot]          = {NULL,           NULL,         PREC_NONE},
    [TokenMinus]        = {parse_unary,    parse_binary, PREC_TERM},
    [TokenPlus]         = {NULL,           parse_binary, PREC_TERM},
    [TokenSemiColon]    = {NULL,           NULL,         PREC_NONE},
    [TokenSlash]        = {NULL,           parse_binary, PREC_FACTOR},
    [TokenStar]         = {NULL,           parse_binary, PREC_FACTOR},
    [TokenNot]          = {parse_unary,    NULL,         PREC_NONE},
    [TokenNotEqual]     = {NULL,           parse_binary, PREC_EQUALITY},
    [TokenEqual]        = {NULL,           NULL,         PREC_NONE},
    [TokenEqualEqual]   = {NULL,           parse_binary, PREC_EQUALITY},
    [TokenGreater]      = {NULL,           parse_binary, PREC_COMPARISON},
    [TokenGreaterEqual] = {NULL,           parse_binary, PREC_COMPARISON},
    [TokenLess]         = {NULL,           parse_binary, PREC_COMPARISON},
    [TokenLessEqual]    = {NULL,           parse_binary, PREC_COMPARISON},
    [TokenIdentifier]   = {parse_variable, NULL,         PREC_NONE},
    [TokenString]       = {parse_string,   NULL,         PREC_NONE},
    [TokenNumber]       = {parse_number,   NULL,         PREC_NONE},
    [TokenAnd]          = {NULL,           parse_and,    PREC_AND},
    [TokenClass]        = {NULL,           NULL,         PREC_NONE},
    [TokenElse]         = {NULL,           NULL,         PREC_NONE},
    [TokenFalse]        = {parse_literal,  NULL,         PREC_NONE},
    [TokenFor]          = {NULL,           NULL,         PREC_NONE},
    [TokenFun]          = {NULL,           NULL,         PREC_NONE},
    [TokenIf]           = {NULL,           NULL,         PREC_NONE},
    [TokenNil]          = {parse_literal,  NULL,         PREC_NONE},
    [TokenOr]           = {NULL,           parse_or,     PREC_OR},
    [TokenPrint]        = {NULL,           NULL,         PREC_NONE},
    [TokenReturn]       = {NULL,           NULL,         PREC_NONE},
    [TokenParent]       = {NULL,           NULL,         PREC_NONE},
    [TokenThis]         = {NULL,           NULL,         PREC_NONE},
    [TokenTrue]         = {parse_literal,  NULL,         PREC_NONE},
    [TokenDef]          = {NULL,           NULL,         PREC_NONE},
    [TokenWhile]        = {NULL,           NULL,         PREC_NONE},
    [TokenError]        = {NULL,           NULL,         PREC_NONE},
    [TokenEof]          = {NULL,           NULL,         PREC_NONE},
};

static ParseRule* get_rule(TokenType type) { return &rules[type]; }

/* ── Error helpers ── */

static void error_at(Parser* p, Token* token, const char* msg) {
    if (p->panic_mode) return;
    p->panic_mode = true;
    fprintf(stderr, "[line %d] Error", token->line);
    if (token->type == TokenEof) {
        fprintf(stderr, " at end");
    } else if (token->type != TokenError) {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }
    fprintf(stderr, ": %s\n", msg);
    p->had_error = true;
    DEBUG_BREAK();
}

static void error_at_current(Parser* p, const char* msg) { error_at(p, &p->current,  msg); }
static void error(Parser* p, const char* msg)             { error_at(p, &p->previous, msg); }

/* ── Public parser API ── */

void parser_init(Parser* p, Scanner* scanner, ByteCode* bytecode) {
    p->scanner    = scanner;
    p->bytecode   = bytecode;
    p->had_error  = false;
    p->panic_mode = false;
}

void parser_advance(Parser* p) {
    p->previous = p->current;
    while (true) {
        p->current = scanner_scan_token(p->scanner);
        if (p->current.type != TokenError) break;
        error_at_current(p, p->current.start);
    }
}

void parser_consume(Parser* p, TokenType type, const char* msg) {
    if (p->current.type == type) { parser_advance(p); return; }
    error_at_current(p, msg);
}

bool parser_match(Parser* p, TokenType type) {
    if (p->current.type != type) return false;
    parser_advance(p);
    return true;
}

bool parser_has_error(Parser* p) { return p->had_error; }

/* ── Emit helpers ── */

void parser_emit_byte(Parser* p, u8 byte) {
    bytecode_write_byte(p->bytecode, byte, p->previous.line);
}
void parser_emit_opcode(Parser* p, OpCode code) {
    bytecode_write_opcode(p->bytecode, code, p->previous.line);
}
void parser_emit_opcodes(Parser* p, OpCode c1, OpCode c2) {
    parser_emit_opcode(p, c1);
    parser_emit_opcode(p, c2);
}
void parser_emit_return(Parser* p) { parser_emit_opcode(p, OP_RETURN); }

void parser_emit_opcode_and_operand(Parser* p, OpCode code, u8 operand) {
    parser_emit_opcode(p, code);
    parser_emit_byte(p, operand);
}

void parser_emit_constant(Parser* p, Value value) {
    int operand = bytecode_add_constant(p->bytecode, value);
    if (operand > UINT8_MAX) { error(p, "Too many constants in one chunk"); return; }
    parser_emit_opcode_and_operand(p, OP_CONSTANT, (u8)operand);
}

int parser_emit_jump(Parser* p, OpCode opcode) {
    parser_emit_opcode(p, opcode);
    parser_emit_byte(p, 0xff);
    parser_emit_byte(p, 0xff);
    return p->bytecode->count - 2;
}

void parser_emit_loop(Parser* p, int loop_start) {
    parser_emit_opcode(p, OP_LOOP);
    int offset = p->bytecode->count - loop_start + 2;
    if (offset > UINT16_MAX) error(p, "Loop body too large");
    parser_emit_byte(p, (u8)((offset >> 8) & 0xff));
    parser_emit_byte(p, (u8)(offset & 0xff));
}

void parser_patch_jump(Parser* p, int offset) {
    int jump = p->bytecode->count - offset - 2;
    if (jump > UINT16_MAX) error(p, "Too much code to jump over");
    p->bytecode->code[offset]     = (u8)((jump >> 8) & 0xff);
    p->bytecode->code[offset + 1] = (u8)(jump & 0xff);
}

/* ── Variable / scope helpers ── */

static u8 identifier_constant(Parser* p, Token* name) {
    return (u8)bytecode_add_constant(p->bytecode,
                                     OBJ_VAL(copy_string(name->start, name->length)));
}

static bool identifiers_equal(Token* a, Token* b) {
    if (a->length != b->length) return false;
    return memcmp(a->start, b->start, (size_t)a->length) == 0;
}

static int resolve_local(Parser* p, Compiler* compiler, Token* name) {
    for (int i = compiler->local_count - 1; i >= 0; i--) {
        Local* local = &compiler->locals[i];
        if (identifiers_equal(name, &local->name)) {
            if (local->depth == -1)
                error(p, "Can't read local variable in its own initializer.");
            return i;
        }
    }
    return -1;
}

static void mark_initialized(void) {
    g_current->locals[g_current->local_count - 1].depth = g_current->scope_depth;
}

static void define_variable(Parser* p, u8 global) {
    if (g_current->scope_depth > 0) { mark_initialized(); return; }
    parser_emit_opcode_and_operand(p, OP_DEFINE_GLOBAL, global);
}

static void declare_variable(Parser* p) {
    if (g_current->scope_depth == 0) return;
    Token* name = &p->previous;
    for (int i = g_current->local_count - 1; i >= 0; i--) {
        Local* local = &g_current->locals[i];
        if (local->depth != -1 && local->depth < g_current->scope_depth) break;
        if (identifiers_equal(name, &local->name))
            error(p, "Already variable with this name in this scope");
    }
    if (g_current->local_count == LOCALS_MAX) {
        error(p, "Too many local variables in function");
        return;
    }
    Local* local = &g_current->locals[g_current->local_count++];
    local->name  = *name;
    local->depth = -1;
}

static u8 parse_variable_name(Parser* p, const char* error_msg) {
    parser_consume(p, TokenIdentifier, error_msg);
    declare_variable(p);
    if (g_current->scope_depth > 0) return 0;
    return identifier_constant(p, &p->previous);
}

static void named_variable(Parser* p, Token name, bool can_assign) {
    OpCode get_op, set_op;
    int    arg = resolve_local(p, g_current, &name);
    if (arg != -1) {
        get_op = OP_GET_LOCAL;
        set_op = OP_SET_LOCAL;
    } else {
        arg    = identifier_constant(p, &name);
        get_op = OP_GET_GLOBAL;
        set_op = OP_SET_GLOBAL;
    }
    if (can_assign && parser_match(p, TokenEqual)) {
        parser_expression(p);
        parser_emit_opcode_and_operand(p, set_op, (u8)arg);
    } else {
        parser_emit_opcode_and_operand(p, get_op, (u8)arg);
    }
}

/* ── Expression parse functions ── */

void parser_parse_precedence(Parser* p, Precedence precedence) {
    parser_advance(p);
    ParseFn prefix = get_rule(p->previous.type)->prefix;
    if (prefix == NULL) { error(p, "Expect expression"); return; }

    bool can_assign = (precedence <= PREC_ASSIGNMENT);
    prefix(p, can_assign);

    while (precedence <= get_rule(p->current.type)->precedence) {
        parser_advance(p);
        ParseFn infix = get_rule(p->previous.type)->infix;
        infix(p, can_assign);
    }
    if (can_assign && parser_match(p, TokenEqual))
        error(p, "Invalid assignment target.");
}

void parser_expression(Parser* p) {
    parser_parse_precedence(p, PREC_ASSIGNMENT);
}

static void parse_number(Parser* p, bool can_assign) {
    (void)can_assign;
    parser_emit_constant(p, NUMBER_VAL(strtod(p->previous.start, NULL)));
}

static void parse_grouping(Parser* p, bool can_assign) {
    (void)can_assign;
    parser_expression(p);
    parser_consume(p, TokenRightParen, "Expect ')' after expression");
}

static void parse_unary(Parser* p, bool can_assign) {
    (void)can_assign;
    TokenType op = p->previous.type;
    parser_parse_precedence(p, PREC_UNARY);
    switch (op) {
    case TokenMinus: parser_emit_opcode(p, OP_NEGATE); break;
    case TokenNot:   parser_emit_opcode(p, OP_NOT);    break;
    default:         break;
    }
}

static void parse_binary(Parser* p, bool can_assign) {
    (void)can_assign;
    TokenType  op   = p->previous.type;
    ParseRule* rule = get_rule(op);
    parser_parse_precedence(p, (Precedence)(rule->precedence + 1));
    switch (op) {
    case TokenPlus:         parser_emit_opcode(p, OP_ADD);                      break;
    case TokenMinus:        parser_emit_opcode(p, OP_SUBTRACT);                 break;
    case TokenStar:         parser_emit_opcode(p, OP_MULTIPLY);                 break;
    case TokenSlash:        parser_emit_opcode(p, OP_DIVIDE);                   break;
    case TokenNotEqual:     parser_emit_opcodes(p, OP_EQUAL,   OP_NOT);         break;
    case TokenEqualEqual:   parser_emit_opcode(p, OP_EQUAL);                    break;
    case TokenGreater:      parser_emit_opcode(p, OP_GREATER);                  break;
    case TokenLess:         parser_emit_opcode(p, OP_LESS);                     break;
    case TokenGreaterEqual: parser_emit_opcodes(p, OP_LESS,    OP_NOT);         break;
    case TokenLessEqual:    parser_emit_opcodes(p, OP_GREATER, OP_NOT);         break;
    default:                break;
    }
}

static void parse_literal(Parser* p, bool can_assign) {
    (void)can_assign;
    switch (p->previous.type) {
    case TokenTrue:  parser_emit_opcode(p, OP_TRUE);  break;
    case TokenFalse: parser_emit_opcode(p, OP_FALSE); break;
    case TokenNil:   parser_emit_opcode(p, OP_NIL);   break;
    default:         break;
    }
}

static void parse_string(Parser* p, bool can_assign) {
    (void)can_assign;
    parser_emit_constant(p,
        OBJ_VAL(copy_string(p->previous.start + 1, p->previous.length - 2)));
}

static void parse_variable(Parser* p, bool can_assign) {
    named_variable(p, p->previous, can_assign);
}

static void parse_and(Parser* p, bool can_assign) {
    (void)can_assign;
    int end_jump = parser_emit_jump(p, OP_JMP_IF_FALSE);
    parser_emit_opcode(p, OP_POP);
    parser_parse_precedence(p, PREC_AND);
    parser_patch_jump(p, end_jump);
}

static void parse_or(Parser* p, bool can_assign) {
    (void)can_assign;
    int else_jump = parser_emit_jump(p, OP_JMP_IF_FALSE);
    int end_jump  = parser_emit_jump(p, OP_JMP);
    parser_patch_jump(p, else_jump);
    parser_emit_opcode(p, OP_POP);
    parser_parse_precedence(p, PREC_OR);
    parser_patch_jump(p, end_jump);
}

/* ── Scope helpers ── */

static void begin_scope(void) { g_current->scope_depth++; }

static void end_scope(Parser* p) {
    g_current->scope_depth--;
    while (g_current->local_count > 0 &&
           g_current->locals[g_current->local_count - 1].depth > g_current->scope_depth) {
        parser_emit_opcode(p, OP_POP);
        g_current->local_count--;
    }
}

static void block(Parser* p) {
    while (p->current.type != TokenRightBrace && p->current.type != TokenEof)
        parser_declaration(p);
    parser_consume(p, TokenRightBrace, "Expect '}' after block");
}

/* ── Statement helpers ── */

static void print_statement(Parser* p) {
    parser_expression(p);
    parser_consume(p, TokenSemiColon, "Expect ';' after value.");
    parser_emit_opcode(p, OP_PRINT);
}

static void expression_statement(Parser* p) {
    parser_expression(p);
    parser_consume(p, TokenSemiColon, "Expect ';' after expression");
    parser_emit_opcode(p, OP_POP);
}

static void if_statement(Parser* p) {
    parser_consume(p, TokenLeftParen,  "Expect '(' after if");
    parser_expression(p);
    parser_consume(p, TokenRightParen, "Expect ')' after condition");

    int then_jump = parser_emit_jump(p, OP_JMP_IF_FALSE);
    parser_emit_opcode(p, OP_POP);
    parser_statement(p);

    int else_jump = parser_emit_jump(p, OP_JMP);
    parser_patch_jump(p, then_jump);
    parser_emit_opcode(p, OP_POP);

    if (parser_match(p, TokenElse)) parser_statement(p);
    parser_patch_jump(p, else_jump);
}

static void while_statement(Parser* p) {
    int loop_start = p->bytecode->count;
    parser_consume(p, TokenLeftParen,  "Expect '(' after while.");
    parser_expression(p);
    parser_consume(p, TokenRightParen, "Expect ')' after condition.");

    int exit_jump = parser_emit_jump(p, OP_JMP_IF_FALSE);
    parser_emit_opcode(p, OP_POP);
    parser_statement(p);
    parser_emit_loop(p, loop_start);

    parser_patch_jump(p, exit_jump);
    parser_emit_opcode(p, OP_POP);
}

static void variable_declaration(Parser* p) {
    u8 global = parse_variable_name(p, "Expect variable name.");
    if (parser_match(p, TokenEqual)) {
        parser_expression(p);
    } else {
        parser_emit_opcode(p, OP_NIL);
    }
    parser_consume(p, TokenSemiColon, "Expect ';' after variable declaration");
    define_variable(p, global);
}

static void for_statement(Parser* p) {
    begin_scope();
    parser_consume(p, TokenLeftParen, "Expect '(' after for.");

    if (parser_match(p, TokenSemiColon)) {
        /* no initializer */
    } else if (parser_match(p, TokenDef)) {
        variable_declaration(p);
    } else {
        expression_statement(p);
    }

    int loop_start = p->bytecode->count;
    int exit_jump  = -1;

    if (!parser_match(p, TokenSemiColon)) {
        parser_expression(p);
        parser_consume(p, TokenSemiColon, "Expect ';' after loop condition");
        exit_jump = parser_emit_jump(p, OP_JMP_IF_FALSE);
        parser_emit_opcode(p, OP_POP);
    }

    if (!parser_match(p, TokenRightParen)) {
        int body_jump       = parser_emit_jump(p, OP_JMP);
        int increment_start = p->bytecode->count;
        parser_expression(p);
        parser_emit_opcode(p, OP_POP);
        parser_consume(p, TokenRightParen, "Expect ')' after for clauses.");
        parser_emit_loop(p, loop_start);
        loop_start = increment_start;
        parser_patch_jump(p, body_jump);
    }

    parser_statement(p);
    parser_emit_loop(p, loop_start);

    if (exit_jump != -1) {
        parser_patch_jump(p, exit_jump);
        parser_emit_opcode(p, OP_POP);
    }
    end_scope(p);
}

static void synchronize(Parser* p) {
    p->panic_mode = false;
    while (p->current.type != TokenEof) {
        if (p->previous.type == TokenSemiColon) return;
        switch (p->current.type) {
        case TokenClass: case TokenFun: case TokenDef: case TokenFor:
        case TokenIf: case TokenWhile: case TokenPrint: case TokenReturn:
            return;
        default:;
        }
        parser_advance(p);
    }
}

/* ── Public statement API ── */

void parser_statement(Parser* p) {
    if      (parser_match(p, TokenPrint))     print_statement(p);
    else if (parser_match(p, TokenFor))        for_statement(p);
    else if (parser_match(p, TokenIf))         if_statement(p);
    else if (parser_match(p, TokenWhile))      while_statement(p);
    else if (parser_match(p, TokenLeftBrace)) { begin_scope(); block(p); end_scope(p); }
    else                                       expression_statement(p);
}

void parser_declaration(Parser* p) {
    if (parser_match(p, TokenDef))
        variable_declaration(p);
    else
        parser_statement(p);
    if (p->panic_mode) synchronize(p);
}
