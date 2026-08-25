#include "pattern_matcher.hpp"
#include <string>

// Простая функция сопоставления паттерна с текстом.
// Поддерживаемые конструкции:
//  - '.'  : любой один символ
//  - '*'  : ноль или более любых символов
//  - '[abc]' : один символ из множества
//  - '\\' : экранирование следующего символа
// Реализация не использует системные библиотеки регулярных выражений,
// реализована рекурсивно с разбором по символам паттерна.

static bool match_here(const std::string &pat, size_t pi, const std::string &s,
                       size_t si);

static bool match_range(const std::string &pat, size_t &pi, char c) {
  // При входе pat[pi] == '['.
  ++pi; // пропустить '['
  bool neg = false;
  if (pi < pat.size() && pat[pi] == '^') {
    neg = true;
    ++pi;
  }
  bool ok = false;
  while (pi < pat.size() && pat[pi] != ']') {
    if (pi + 2 < pat.size() && pat[pi + 1] == '-') {
      char a = pat[pi];
      char b = pat[pi + 2];
      if (a <= c && c <= b)
        ok = true;
      pi += 3;
    } else {
      if (pat[pi] == c)
        ok = true;
      ++pi;
    }
  }
  if (pi < pat.size() && pat[pi] == ']')
    ++pi;
  return neg ? !ok : ok;
}

static bool match_star(const std::string &pat, size_t pi, const std::string &s,
                       size_t si) {
  // '*' соответствует нулю или более любых символов.
  // Проверить все возможные длины (жадный перебор через рекурсию).
  if (pi == pat.size())
    return true; // Завершающий '*' соответствует оставшейся строке.
  for (size_t k = si; k <= s.size(); ++k) {
    if (match_here(pat, pi, s, k))
      return true;
  }
  return false;
}

static bool match_here(const std::string &pat, size_t pi, const std::string &s,
                       size_t si) {
  if (pi == pat.size())
    return si == s.size();
  if (pat[pi] == '*') {
    return match_star(pat, pi + 1, s, si);
  }
  if (si == s.size())
    return false;
  if (pat[pi] == '.') {
    return match_here(pat, pi + 1, s, si + 1);
  }
  if (pat[pi] == '[') {
    size_t saved = pi;
    bool ok = match_range(pat, pi, s[si]);
    if (!ok)
      return false;
    return match_here(pat, pi, s, si + 1);
  }
  if (pat[pi] == '\\') {
    // Экранировать следующий символ.
    ++pi;
    if (pi == pat.size())
      return false;
    if (pat[pi] != s[si])
      return false;
    return match_here(pat, pi + 1, s, si + 1);
  }
  // Обычный символ.
  if (pat[pi] == s[si])
    return match_here(pat, pi + 1, s, si + 1);
  return false;
}

bool pattern_match(const std::string &pattern, const std::string &text) {
  return match_here(pattern, 0, text, 0);
}
