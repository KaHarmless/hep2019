#include <iostream>
#include <vector>
#include <cstring>

#include "LCS.hpp"

template <>
struct LCS<std::string>::concat {
  std::string operator () (char c, const std::string& s)
  { return c + s; }
};

int main()
{
  LCS<std::string> lcs { "AGCAT", "GAC" };

  std::cout << "\e[4mtable\e[0m:\n"
            << lcs
            << std::endl;

  std::cout << "\e[4mLCSs\e[0m:\n";
  for (auto& s:lcs.get())
    std::cout << s << std::endl;
  std::cout << std::endl;

  std::cout << "\e[4mdiff\e[0m:\n";
  lcs.diff(std::cout);
}
