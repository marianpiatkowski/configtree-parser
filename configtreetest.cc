#include <iostream>

#include "configtreeparser.hh"

#if HAVE_EIGEN
#include <Eigen/Core>
#include <Eigen/Dense>
#endif // HAVE_EIGEN

template<class P>
void testparam(const P & p)
{
  // try accessing key
  assert(p.template get<int>("x1") == 1);
  assert(p.template get<double>("x1") == 1.0);
  assert(p.template get<std::string>("x2") == "hallo");
  assert(p.template get<bool>("x3") == false);
  // try reading array like structures
  std::vector<unsigned int>
    array1 = p.template get< std::vector<unsigned int> >("array");
  std::array<unsigned int, 8>
    array2 = p.template get< std::array<unsigned int, 8> >("array");
#if HAVE_EIGEN
  Eigen::Matrix<unsigned int, 8, 1>
    array3 = p.template get< Eigen::Matrix<unsigned int, 8, 1> >("array");
#endif // HAVE_EIGEN
  assert(array1.size() == 8);
  for (unsigned int i=0; i<8; i++)
  {
    assert(array1[i] == i+1);
    assert(array2[i] == i+1);
#if HAVE_EIGEN
    assert(array3[i] == i+1);
#endif // HAVE_EIGEN
  }
  // try accessing subtree
  p.sub("Foo");
  p.sub("Foo").template get<std::string>("peng");
  // check hasSub and hasKey
  assert(p.hasSub("Foo"));
  assert(not p.hasSub("x1"));
  assert(p.hasKey("x1"));
  assert(not p.hasKey("Foo"));
  // try accessing inexistent key
  try
  {
    p.template get<int>("bar");
    // throw exception not to be caught by the block
    throw std::runtime_error("Failed to detect missing key");
  }
  catch (std::range_error& r) {}
  // try accessing inexistent subtree in throwing mode
  try
  {
    p.sub("bar",true);
    throw std::runtime_error("Failed to detect missing subtree");
  }
  catch (std::range_error& r) {}
  // try accessing inexistent subtree in non-throwing mode
  p.sub("bar");
  // try accessing inexistent subtree that shadows a value key
  try
  {
    p.sub("x1.bar");
    throw std::runtime_error("Succeeded to access non-existent subtree that shadows a value key");
  }
  catch (std::range_error& r) {}
  // try accessing key as subtree
  try
  {
    p.sub("x1");
    throw std::runtime_error("Succeeded to access key as subtree");
  }
  catch (std::range_error& r) {}
  // try accessing subtree as key
  try
  {
    p.template get<double>("Foo");
    throw std::runtime_error("Succeeded to access subtree as key");
  }
  catch (std::range_error& r) {}
}

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

void testOptionsParserResults(std::vector<std::string> args,
                              const std::vector<std::string> & keywords,
                              unsigned int required,
                              bool allow_more,
                              bool overwrite,
                              std::string foo, std::string bar,
                              const std::string referr = "")
{
  ConfigTree pt;
  try
  {
    char * argv[10];
    for (std::size_t i = 0; i < args.size(); ++i)
      argv[i] = &args[i][0];
    ConfigTreeParser::readNamedOptions(args.size(), argv, pt, keywords, required, allow_more, overwrite);
    assert(referr == "");
  }
  catch (const std::range_error & e)
  {
    std::string err = e.what();
    err = err.substr(0, err.find('\n'));
    assert(referr == err);
  }
  if (foo not_eq "" and foo not_eq pt.get<std::string>("foo"))
  {
    std::ostringstream message;
    message << "Options parser failed... foo = "
            << pt.get<std::string>("foo") << " != " << foo;
    throw std::runtime_error(message.str());
  }
  if (bar not_eq "" and bar not_eq pt.get<std::string>("bar"))
  {
    std::ostringstream message;
    message << "Options parser failed... bar = "
            << pt.get<std::string>("bar") << " != " << bar;
    throw std::runtime_error(message.str());
  }
}

void testOptionsParser()
{
  std::vector<std::string> keywords = { "foo", "bar" };
  // check normal behaviour
  {
    std::vector<std::string> args = { "progname", "--bar=ligapokal", "peng", "--bar=ligapokal", "--argh=other"};
    testOptionsParserResults(args,keywords,keywords.size(),true,true,"peng","ligapokal",
      "" /* no error */ );
  }
  // bail out on overwrite
  {
    std::vector<std::string> args = { "progname", "--bar=ligapokal", "peng", "--bar=ligapokal", "--argh=other"};
    testOptionsParserResults(args,keywords,keywords.size(),true,false,"peng","ligapokal",
      "parameter bar already specified");
  }
  // bail out on unknown options
  {
    std::vector<std::string> args = { "progname", "--bar=ligapokal", "peng", "--bar=ligapokal", "--argh=other"};
    testOptionsParserResults(args,keywords,keywords.size(),false,true,"peng","ligapokal",
      "unknown parameter argh");
  }
  // bail out on missing parameter
  {
    std::vector<std::string> args = { "progname", "--bar=ligapokal"};
    testOptionsParserResults(args,keywords,keywords.size(),true,true,"","ligapokal",
      "missing parameter(s) ...  foo");
  }
  // check optional parameter
  {
    std::vector<std::string> args = { "progname", "--foo=peng"};
    testOptionsParserResults(args,keywords,1,true,true,"peng","",
      "" /* no error */);
  }
  // check optional parameter, but bail out on missing parameter
  {
    std::vector<std::string> args = { "progname", "--bar=ligapokal"};
    testOptionsParserResults(args,keywords,1,true,true,"","ligapokal",
      "missing parameter(s) ...  foo");
  }
  // bail out on too many parameters
  {
    std::vector<std::string> args = { "progname", "peng", "ligapokal", "hurz"};
    testOptionsParserResults(args,keywords,keywords.size(),true,true,"peng","ligapokal",
      "superfluous unnamed parameter");
  }
  // bail out on missing value
  {
    std::vector<std::string> args = { "progname", "--foo=peng", "--bar=ligapokal", "--hurz"};
    testOptionsParserResults(args,keywords,keywords.size(),true,true,"peng","ligapokal",
      "value missing for parameter --hurz");
  }
}

void check_recursiveTreeCompare(const ConfigTree & p1,
                                const ConfigTree & p2)
{
  assert(p1.getValueKeys() == p2.getValueKeys());
  assert(p1.getSubKeys() == p2.getSubKeys());
  typedef ConfigTree::KeyVector::const_iterator Iterator;
  for (Iterator it = p1.getValueKeys().begin();
       it not_eq p1.getValueKeys().end(); ++it)
    assert(p1[*it] == p2[*it]);
  for (Iterator it = p1.getSubKeys().begin();
       it not_eq p1.getSubKeys().end(); ++it)
    check_recursiveTreeCompare(p1.sub(*it), p2.sub(*it));
}

// test report method and read back in
void testReport()
{
  std::stringstream s;
  s << "foo.i = 1 \n foo.bar.peng = hurz";
  ConfigTree ptree;
  ConfigTreeParser::readINITree(s, ptree);

  std::stringstream s2;
  ptree.report(s2);
  ConfigTree ptree2;
  ConfigTreeParser::readINITree(s2, ptree2);
  check_recursiveTreeCompare(ptree, ptree2);
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

  // more const tests
  testparam<ConfigTree>(c);

  // check the command line parser
  testOptionsParser();

  // check report
  testReport();

  return 0;
}
