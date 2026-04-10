#include "scanner.h"
#include <string.h>

void scanner_init(Scanner* scanner, const char* source) {
    scanner->start   = source;
    scanner->current = source;
    scanner->line    = 1;
}

static bool is_at_end(Scanner* s) { return *s->current == '\0'; }

static Token make_token(Scanner* s, TokenType type) {
    Token t;
    t.type   = type;
    t.start  = s->start;
    t.length = (int)(s->current - s->start);
    t.line   = s->line;
    return t;
}

static Token error_token(Scanner* s, const char* message) {
    Token t;
    t.type   = TokenError;
    t.start  = message;
    t.length = (int)strlen(message);
    t.line   = s->line;
    return t;
}

static char advance(Scanner* s) {
    s->current++;
    return s->current[-1];
}

static char peek(Scanner* s)      { return *s->current; }
static char peek_next(Scanner* s) { return is_at_end(s) ? '\0' : s->current[1]; }

static bool match(Scanner* s, char expected) {
    if (is_at_end(s) || *s->current != expected) return false;
    s->current++;
    return true;
}

static bool is_digit(char c) { return c >= '0' && c <= '9'; }
static bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static void skip_whitespace(Scanner* s) {
    while (true) {
        switch (peek(s)) {
        case ' ': case '\r': case '\t': advance(s); break;
        case '\n': s->line++; advance(s); break;
        case '/':
            if (peek_next(s) == '/') {
                while (peek(s) != '\n' && !is_at_end(s)) advance(s);
            } else {
                return;
            }
            break;
        default: return;
        }
    }
}

static Token string_token(Scanner* s) {
    while (peek(s) != '"' && !is_at_end(s)) {
        if (peek(s) == '\n') s->line++;
        advance(s);
    }
    if (is_at_end(s)) return error_token(s, "Unterminated string");
    advance(s); /* closing quote */
    return make_token(s, TokenString);
}

static Token number_token(Scanner* s) {
    while (is_digit(peek(s))) advance(s);
    if (peek(s) == '.' && is_digit(peek_next(s))) {
        advance(s);
        while (is_digit(peek(s))) advance(s);
    }
    return make_token(s, TokenNumber);
}

static TokenType check_keyword(Scanner* s, int start, int length,
                                const char* rest, TokenType type) {
    if (s->current - s->start == start + length &&
        memcmp(s->start + start, rest, (size_t)length) == 0) {
        return type;
    }
    return TokenIdentifier;
}

static TokenType identifier_type(Scanner* s) {
    switch (s->start[0]) {
    case 'a': return check_keyword(s, 1, 2, "nd",    TokenAnd);
    case 'c': return check_keyword(s, 1, 4, "lass",  TokenClass);
    case 'd': return check_keyword(s, 1, 2, "ef",    TokenDef);
    case 'e': return check_keyword(s, 1, 3, "lse",   TokenElse);
    case 'f':
        if (s->current - s->start > 1) {
            switch (s->start[1]) {
            case 'a': return check_keyword(s, 2, 3, "lse", TokenFalse);
            case 'o': return check_keyword(s, 2, 1, "r",   TokenFor);
            case 'u': return check_keyword(s, 2, 1, "n",   TokenFun);
            }
        }
        break;
    case 'i': return check_keyword(s, 1, 1, "f",     TokenIf);
    case 'n': return check_keyword(s, 1, 2, "il",    TokenNil);
    case 'o': return check_keyword(s, 1, 1, "r",     TokenOr);
    case 'p':
        if (s->current - s->start > 1) {
            switch (s->start[1]) {
            case 'r': return check_keyword(s, 2, 3, "int",  TokenPrint);
            case 'a': return check_keyword(s, 2, 4, "rent", TokenParent); /* fixed: was TokenPrint */
            }
        }
        break;
    case 'r': return check_keyword(s, 1, 5, "eturn", TokenReturn);
    case 't':
        if (s->current - s->start > 1) {
            switch (s->start[1]) {
            case 'h': return check_keyword(s, 2, 2, "is", TokenThis);
            case 'r': return check_keyword(s, 2, 2, "ue", TokenTrue);
            }
        }
        break;
    case 'w': return check_keyword(s, 1, 4, "hile",  TokenWhile);
    }
    return TokenIdentifier;
}

static Token identifier_token(Scanner* s) {
    while (is_alpha(peek(s)) || is_digit(peek(s))) advance(s);
    return make_token(s, identifier_type(s));
}

Token scanner_scan_token(Scanner* s) {
    skip_whitespace(s);
    s->start = s->current;
    if (is_at_end(s)) return make_token(s, TokenEof);

    char c = advance(s);
    if (is_alpha(c)) return identifier_token(s);
    if (is_digit(c)) return number_token(s);

    switch (c) {
    case '(': return make_token(s, TokenLeftParen);
    case ')': return make_token(s, TokenRightParen);
    case '{': return make_token(s, TokenLeftBrace);
    case '}': return make_token(s, TokenRightBrace);
    case ';': return make_token(s, TokenSemiColon);
    case ',': return make_token(s, TokenComma);
    case '.': return make_token(s, TokenDot);
    case '-': return make_token(s, TokenMinus);
    case '+': return make_token(s, TokenPlus);
    case '/': return make_token(s, TokenSlash);
    case '*': return make_token(s, TokenStar);
    case '!': return make_token(s, match(s, '=') ? TokenNotEqual    : TokenNot);
    case '=': return make_token(s, match(s, '=') ? TokenEqualEqual  : TokenEqual);
    case '<': return make_token(s, match(s, '=') ? TokenLessEqual   : TokenLess);
    case '>': return make_token(s, match(s, '=') ? TokenGreaterEqual : TokenGreater);
    case '"': return string_token(s);
    default:  break;
    }

    return error_token(s, "Unexpected character");
}
