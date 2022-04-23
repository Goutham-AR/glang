#pragma once

#include "common.hh"
#include <string_view>

enum TokenType {
    TokenEof,
    TokenError,

    // single char
    TokenLeftParen,
    TokenRightParen,
    TokenLeftBrace,
    TokenRightBrace,
    TokenComma,
    TokenDot,
    TokenMinus,
    TokenPlus,
    TokenSemiColon,
    TokenSlash,
    TokenStar,

    // one or two char
    TokenNot,
    TokenNotEqual,
    TokenEqual,
    TokenEqualEqual,
    TokenLess,
    TokenLessEqual,
    TokenGreater,
    TokenGreaterEqual,

    // literals
    TokenIdentifier,
    TokenString,
    TokenNumber,

    // keywords
    TokenAnd,
    TokenClass,
    TokenElse,
    TokenFalse,
    TokenFor,
    TokenFun,
    TokenIf,
    TokenNil,
    TokenOr,
    TokenPrint,
    TokenReturn,
    TokenParent,
    TokenThis,
    TokenTrue,
    TokenDef,
    TokenWhile

};

struct Token {
    TokenType type;
    std::string_view name;
    int line;
};

class Scanner {
public:
    explicit Scanner(const std::string& source);
    ~Scanner() = default;

    void init(const std::string& source);
    Token scanToken();

private:
    bool isAtEnd();
    Token makeToken(TokenType type);
    Token errorToken(std::string_view errorMsg);
    char advance();
    bool match(char c);
    void skipWhiteSpace();
    char peek();
    char peekNext();

    Token stringToken();
    bool isDigit(char c);
    Token numberToken();

    bool isAlpha(char c);
    Token identifier();

    TokenType identifierType();
    TokenType checkKeyword(int start, int length, const char* rest, TokenType type);

private:
    std::string source_;
    const char* start_;
    const char* current_;
    int line_;
};