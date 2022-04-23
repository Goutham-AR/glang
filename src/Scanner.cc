#include "Scanner.hh"

Scanner::Scanner(const std::string& source)
    : source_{source},
      start_{source_.c_str()},
      current_{source_.c_str()},
      line_{1} {
}

void Scanner::init(const std::string& source) {
    source_ = source;
    current_ = start_ = source.c_str();
    line_ = 1;
}

Token Scanner::scanToken() {
    skipWhiteSpace();

    start_ = current_;

    if (isAtEnd()) return makeToken(TokenEof);

    char c = advance();

    if (isAlpha(c)) return identifier();
    if (isDigit(c)) return numberToken();

    switch (c) {

    // single char
    case '(':
        return makeToken(TokenLeftParen);
    case ')':
        return makeToken(TokenRightParen);
    case '{':
        return makeToken(TokenLeftBrace);
    case '}':
        return makeToken(TokenRightBrace);
    case ';':
        return makeToken(TokenSemiColon);
    case ',':
        return makeToken(TokenComma);
    case '.':
        return makeToken(TokenDot);
    case '-':
        return makeToken(TokenMinus);
    case '+':
        return makeToken(TokenPlus);
    case '/':
        return makeToken(TokenSlash);
    case '*':
        return makeToken(TokenStar);

    // 1 or 2 char
    case '!':
        return makeToken(match('=') ? TokenNotEqual : TokenNot);
    case '=':
        return makeToken(match('=') ? TokenEqualEqual : TokenEqual);
    case '<':
        return makeToken(match('=') ? TokenLessEqual : TokenLess);
    case '>':
        return makeToken(match('=') ? TokenGreaterEqual : TokenGreater);

    case '"':
        return stringToken();

    case '\n':
        ++line_;
        advance();
        break;

    default:
        break;
    }

    return errorToken("Unexpected character");
}

Token Scanner::stringToken() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') ++line_;
        advance();
    }

    if (isAtEnd()) return errorToken("Unterminated string");

    advance(); // consume closing quote
    return makeToken(TokenString);
}

bool Scanner::isAlpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}
Token Scanner::identifier() {
    while (isAlpha(peek()) || isDigit(peek()))
        advance();

    return makeToken(identifierType());
}

TokenType Scanner::identifierType() {
    switch (start_[0]) {
    case 'a':
        return checkKeyword(1, 2, "nd", TokenAnd);
    case 'c':
        return checkKeyword(1, 4, "lass", TokenClass);
    case 'd':
        return checkKeyword(1, 2, "ef", TokenDef);
    case 'e':
        return checkKeyword(1, 3, "lse", TokenElse);
    case 'f':
        if (current_ - start_ > 1) {
            switch (start_[1]) {
            case 'a':
                return checkKeyword(2, 3, "lse", TokenFalse);
            case 'o':
                return checkKeyword(2, 1, "r", TokenFor);
            case 'u':
                return checkKeyword(2, 1, "n", TokenFun);
            }
        }
        break;
    case 'i':
        return checkKeyword(1, 1, "f", TokenIf);
    case 'n':
        return checkKeyword(1, 2, "il", TokenNil);
    case 'o':
        return checkKeyword(1, 1, "r", TokenOr);
    case 'p':
        if (current_ - start_ > 1) {
            switch (start_[1]) {
            case 'r':
                return checkKeyword(2, 3, "int", TokenPrint);
            case 'a':
                return checkKeyword(2, 4, "rent", TokenPrint);
            }
        }
        break;
    case 'r':
        return checkKeyword(1, 5, "eturn", TokenReturn);
    case 't':
        if (current_ - start_ > 1) {
            switch (start_[1]) {
            case 'h':
                return checkKeyword(2, 2, "is", TokenThis);
            case 'r':
                return checkKeyword(2, 2, "ue", TokenTrue);
            }
        }
        break;
    case 'w':
        return checkKeyword(1, 4, "hile", TokenWhile);
    }

    return TokenIdentifier;
}

TokenType Scanner::checkKeyword(int start, int length, const char* rest, TokenType type) {
    if (current_ - start_ == start + length && memcmp(start_ + start, rest, length) == 0) {
        return type;
    }

    return TokenIdentifier;
}
bool Scanner::isAtEnd() {
    return *current_ == '\0';
}
bool Scanner::match(char c) {
    if (isAtEnd()) return false;
    if (*current_ != c) return false;
    ++current_;
    return true;
}

void Scanner::skipWhiteSpace() {
    while (true) {
        auto c = peek();
        switch (c) {
        case ' ':
        case '\r':
        case '\t':
            advance();
            break;
        case '/':
            if (peekNext() == '/') {
                while (peek() != '\n' && !isAtEnd())
                    advance();
            } else {
                return;
            }
            break;

        default:
            return;
        }
    }
}

bool Scanner::isDigit(char c) {
    return c >= '0' && c <= '9';
}
Token Scanner::numberToken() {
    while (isDigit(peek()))
        advance();

    if (peek() == '.' && isDigit(peekNext())) {
        advance(); // consume '.'

        while (isDigit(peek()))
            advance();
    }

    return makeToken(TokenNumber);
}

char Scanner::peek() {
    return *current_;
}
char Scanner::peekNext() {
    if (isAtEnd()) return '\0';
    return current_[1];
}
Token Scanner::makeToken(TokenType type) {
    auto length = static_cast<int>(current_ - start_);

    return Token{
        .type = type,
        .name = std::string_view(start_, length),
        .line = line_,
    };
}

Token Scanner::errorToken(std::string_view errorMsg) {
    return Token{
        .type = TokenError,
        .name = errorMsg,
        .line = line_,
    };
}

char Scanner::advance() {
    ++current_;
    return current_[-1];
}
