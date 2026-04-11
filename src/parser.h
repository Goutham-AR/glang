#ifndef SOLV_PARSER_H
#define SOLV_PARSER_H

#include "common.h"
#include "scanner.h"
#include "bytecode.h"
#include "compiler.h"

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,
    PREC_OR,
    PREC_AND,
    PREC_EQUALITY,
    PREC_COMPARISON,
    PREC_TERM,
    PREC_FACTOR,
    PREC_UNARY,
    PREC_CALL,
    PREC_PRIMARY
} Precedence;

/* Forward declare Parser before ParseFn (which takes Parser*) */
typedef struct Parser Parser;
typedef void (*ParseFn)(Parser* parser, bool can_assign);

typedef struct {
    ParseFn    prefix;
    ParseFn    infix;
    Precedence precedence;
} ParseRule;

struct Parser {
    Scanner*  scanner;
    ByteCode* bytecode;
    Token     current;
    Token     previous;
    bool      had_error;
    bool      panic_mode;
};

void parser_init(Parser* parser, Scanner* scanner, ByteCode* bytecode);
void parser_advance(Parser* parser);
void parser_consume(Parser* parser, TokenType type, const char* msg);
bool parser_match(Parser* parser, TokenType type);
bool parser_has_error(Parser* parser);

void parser_emit_byte(Parser* parser, u8 byte);
void parser_emit_return(Parser* parser);
void parser_emit_opcode(Parser* parser, OpCode code);
void parser_emit_opcodes(Parser* parser, OpCode code1, OpCode code2);
void parser_emit_opcode_and_operand(Parser* parser, OpCode code, u8 operand);
void parser_emit_constant(Parser* parser, Value value);
int  parser_emit_jump(Parser* parser, OpCode opcode);
void parser_emit_loop(Parser* parser, int loop_start);
void parser_patch_jump(Parser* parser, int offset);

void parser_expression(Parser* parser);
void parser_parse_precedence(Parser* parser, Precedence precedence);
void parser_declaration(Parser* parser);
void parser_statement(Parser* parser);

#endif /* SOLV_PARSER_H */
