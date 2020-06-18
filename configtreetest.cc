#include <iostream>

#include "configtreeparser.hh"

int main()
{
  // read config
  std::stringstream s;
  s << "x1 = 1 # comment\n"
    << "x2 = hallo\n"
    << "x3 = no\n"
    << "array = 1   2 3 4 5\t6 7 8\n"
    << "\n"
    << "[Foo]\n"
    << "peng = ligapokal\n";

  ConfigTree c;
  ConfigTreeParser::readINITree(s,c);
  return 0;
}
