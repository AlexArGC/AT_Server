#include "dictionary.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>

std::vector<Entry> load_dictionary(const std::string &path) {
  std::vector<Entry> out;
  std::ifstream in(path);
  if (!in)
    return out;
  std::string line;
  while (std::getline(in, line)) {
    // убрать пробелы по краям
    auto l = line;
    l.erase(l.begin(), std::find_if(l.begin(), l.end(), [](unsigned char ch) {
              return !std::isspace(ch);
            }));
    l.erase(std::find_if(l.rbegin(), l.rend(),
                         [](unsigned char ch) { return !std::isspace(ch); })
                .base(),
            l.end());
    if (l.empty())
      continue;
    if (l[0] == '#')
      continue;
    // Разделить строку на паттерн и ответ по последнему '=' или ','.
    // '=' может быть частью AT-паттерна, например AT+CSQ=[0-9][0-9]=OK.
    size_t eq = l.rfind('=');
    size_t cm = l.find(',');
    size_t pos = std::string::npos;
    if (eq != std::string::npos)
      pos = eq;
    else if (cm != std::string::npos)
      pos = cm;
    if (pos == std::string::npos)
      continue;
    Entry e;
    e.pattern = l.substr(0, pos);
    e.answer = l.substr(pos + 1);
    // убрать пробелы вокруг паттерна и ответа
    auto trim = [](std::string &s) {
      s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                return !std::isspace(ch);
              }));
      s.erase(std::find_if(s.rbegin(), s.rend(),
                           [](unsigned char ch) { return !std::isspace(ch); })
                  .base(),
              s.end());
    };
    trim(e.pattern);
    trim(e.answer);
    // распаковать управляющие последовательности в ответах, например
    // "\n" -> "\r\n" для корректного перевода строки в терминале
    auto unescape = [](const std::string &s) {
      std::string r;
      r.reserve(s.size());
      for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '\\' && i + 1 < s.size()) {
          char n = s[++i];
          switch (n) {
          case 'n':
            r.append("\r\n");
            break;
          case 'r':
            r.push_back('\r');
            break;
          case 't':
            r.push_back('\t');
            break;
          case '\\':
            r.push_back('\\');
            break;
          default:
            r.push_back(n);
            break;
          }
        } else {
          r.push_back(c);
        }
      }
      return r;
    };
    e.answer = unescape(e.answer);
    out.push_back(e);
  }
  return out;
}
