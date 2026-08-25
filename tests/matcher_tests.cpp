#include "pattern_matcher.hpp"
#include <cassert>
#include <iostream>

int main() {
    // Обычные символы и точное совпадение.
    assert(pattern_match("AT", "AT"));
    assert(!pattern_match("AT", "AT\r"));
    assert(!pattern_match("AT", "ATI"));
    assert(pattern_match("ATI", "ATI"));
    assert(pattern_match("AT+COPS", "AT+COPS"));
    assert(pattern_match("AT+CPIN", "AT+CPIN"));

    // Наборы символов и диапазоны.
    assert(pattern_match("ATE[01]", "ATE0"));
    assert(pattern_match("ATE[01]", "ATE1"));
    assert(!pattern_match("ATE[01]", "ATE2"));
    assert(pattern_match("AT+CSQ=[0-9][0-9]", "AT+CSQ=11"));
    assert(pattern_match("AT+CSQ=[0-9][0-9]", "AT+CSQ=99"));
    assert(!pattern_match("AT+CSQ=[0-9][0-9]", "AT+CSQ=1"));
    assert(!pattern_match("AT+CSQ=[0-9][0-9]", "AT+CSQ=1A"));
    assert(pattern_match("AT+TEST[0-9A-F]", "AT+TESTF"));
    assert(pattern_match("AT+TEST[0-9A-F]", "AT+TEST7"));
    assert(!pattern_match("AT+TEST[0-9A-F]", "AT+TESTG"));
    assert(pattern_match("AT+CMD[^0-9]", "AT+CMDX"));
    assert(!pattern_match("AT+CMD[^0-9]", "AT+CMD5"));

    // '*' соответствует нулю или более символов, '.' — одному символу.
    assert(pattern_match("A*E", "AE"));
    assert(pattern_match("A*E", "AbcdE"));
    assert(!pattern_match("A*E", "AbcdF"));
    assert(pattern_match("*", ""));
    assert(pattern_match("*", "anything"));
    assert(pattern_match("A.E", "AbE"));
    assert(!pattern_match("A.E", "AE"));
    assert(!pattern_match("A.E", "AbcE"));

    // Обратная косая черта экранирует специальные символы паттерна.
    assert(pattern_match(R"(A\*B)", "A*B"));
    assert(pattern_match(R"(A\.B)", "A.B"));
    assert(pattern_match(R"(A\[B)", "A[B"));
    assert(!pattern_match(R"(A\*B)", "AXXB"));
    assert(!pattern_match(R"(A\)", "A"));

    std::cout << "All matcher tests passed\n";
    return 0;
}
