#pragma once

#include <string>
#include <vector>

struct Entry {
  std::string pattern;
  std::string answer;
};

std::vector<Entry> load_dictionary(const std::string &path);
