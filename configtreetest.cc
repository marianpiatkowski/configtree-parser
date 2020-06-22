#include <iostream>

#include "configtreeparser.hh"

#if HAVE_EIGEN
#include <Eigen/Core>
#include <Eigen/Dense>
#endif // HAVE_EIGEN

template<class P>
void testmodify(P parameterSet)
{
  parameterSet["testDouble"] = "3.14";
  parameterSet["testInt"]    = "42";
  parameterSet["testString"] = "Hallo Welt!";
  parameterSet["testVector"] = "2 3 5 7 11";
  parameterSet.sub("Foo")["bar"] = "2";

  double testDouble      = parameterSet.template get<double>("testDouble");
  int testInt            = parameterSet.template get<int>("testInt");
  ++testDouble;
  ++testInt;
  std::string testString = parameterSet.template get<std::string>("testString");
#if HAVE_EIGEN
  typedef Eigen::Matrix<unsigned,5,1> EVector;
  EVector testEVector    = parameterSet.template get<EVector>("testVector");
#endif // HAVE_EIGEN
  typedef std::vector<unsigned> SVector;
  SVector testSVector    = parameterSet.template get<SVector>("testVector");
  if(testSVector.size() not_eq 5)
  {
    std::ostringstream message;
    message << "Testing std::vector<unsigned>: expected size()==5, got size()==" << testSVector.size();
    throw std::runtime_error(message.str());
  }
#if HAVE_EIGEN
  for(unsigned i = 0; i < 5; ++i)
    if(testEVector[i] not_eq testSVector[i])
    {
      std::ostringstream message;
      message << "testEVector[" << i << "]==" << testEVector[i] << " but "
              << "testSVector[" << i << "]==" << testSVector[i];
      throw std::runtime_error(message.str());
    }
#endif // HAVE_EIGEN
  if (parameterSet.template get<std::string>("Foo.bar") != "2")
  {
    std::ostringstream message;
    message << "Failed to write subtree entry";
    throw std::range_error(message.str());
  }
  if (parameterSet.sub("Foo").template get<std::string>("bar") != "2")
  {
    std::ostringstream message;
    message << "Failed to write subtree entry";
    throw std::range_error(message.str());
  }
}

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

  // test modifying and reading
  testmodify<ConfigTree>(c);
  try
  {
    c.get<int>("testInt");
    // throw exception not to be caught by the block
    throw std::runtime_error("Unexpected shallow copy of ParameterTree");
  }
  catch(std::range_error& e) {}

  return 0;
}
