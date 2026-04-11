#ifndef SOLV_SCANNER_H
#define SOLV_SCANNER_H

#include "common.h"

typedef enum {
    /* single character tokens */
    TokenLeftParen, TokenRightParen,
    TokenLeftBrace, TokenRightBrace,
    TokenComma, TokenDot,
    TokenMinus, TokenPlus,
    TokenSemiColon, TokenSlash, TokenStar,
    /* one or two character tokens */
    TokenNot, TokenNotEqual,
    TokenEqual, TokenEqualEqual,
    TokenLess, TokenLessEqual,
    TokenGreater, TokenGreaterEqual,
    /* literals */
    TokenIdentifier, TokenString, TokenNumber,
    /* keywords */
    TokenAnd, TokenClass, TokenElse, TokenFalse,
    TokenFor, TokenFun, TokenIf, TokenNil,
    TokenOr, TokenPrint, TokenReturn,
    TokenParent, TokenThis, TokenTrue,
    TokenDef, TokenWhile,
    TokenEof,
    TokenError
} TokenType;

typedef struct {
    TokenType   type;
    const char* start;   /* pointer into source (not NUL-terminated) */
    int         length;
    int         line;
} Token;

typedef struct {
    const char* start;
    const char* current;
    int         line;
} Scanner;

void  scanner_init(Scanner* scanner, const char* source);
Token scanner_scan_token(Scanner* scanner);

#endif /* SOLV_SCANNER_H */
