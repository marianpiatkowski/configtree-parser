// -*- tab-width: 2; indent-tabs-mode: nil -*-
// vi: set et ts=2 sw=2 sts=2:
#ifndef CONFIGTREE_HH
#define CONFIGTREE_HH

#include <sstream>
#include <map>

/** \brief Hierarchical structure of string parameters
 * \ingroup Common
 */
class ConfigTree
{
  // class providing a single static parse() function, used by the
  // generic get() method
  // template specializations follow below
  template<typename T>
  struct Parser;

public:

  /** \brief storage for key lists
   */
  typedef std::vector<std::string> KeyVector;

  /** \brief Create new empty ParameterTree
   */
  ConfigTree()
  {}


  /** \brief test for key
   *
   * Tests whether given key exists.
   *
   * \param key key name
   * \return true if key exists in structure, otherwise false
   */
  bool hasKey(const std::string& key) const
  {
    std::string::size_type dot = key.find(".");

    if (dot not_eq std::string::npos)
    {
      std::string prefix = key.substr(0,dot);
      if (subs_.count(prefix) == 0)
        return false;

      if (values_.count(prefix) > 0)
      {
        std::ostringstream message;
        message << "key " << prefix << " occurs as value and as subtree";
        throw std::range_error(message.str());
      }

      const ConfigTree& s = sub(prefix);
      return s.hasKey(key.substr(dot+1));
    }
    else
      if (values_.count(key) not_eq 0)
      {
        if (subs_.count(key) > 0)
        {
          std::ostringstream message;
          message << "key " << key << " occurs as value and as subtree";
          throw std::range_error(message.str());
        }
        return true;
      }
      else
        return false;

  }


  /** \brief test for substructure
   *
   * Tests whether given substructure exists.
   *
   * \param key substructure name
   * \return true if substructure exists in structure, otherwise false
   */
  bool hasSub(const std::string& key) const
  {
    std::string::size_type dot = key.find(".");

    if (dot not_eq std::string::npos)
    {
      std::string prefix = key.substr(0,dot);
      if (subs_.count(prefix) == 0)
        return false;

      if (values_.count(prefix) > 0)
      {
        std::ostringstream message;
        message << "key " << prefix << " occurs as value and as subtree";
        throw std::range_error(message.str());
      }

      const ConfigTree& s = sub(prefix);
      return s.hasSub(key.substr(dot+1));
    }
    else
      if (subs_.count(key) not_eq 0)
      {
        if (values_.count(key) > 0)
        {
          std::ostringstream message;
          message << "key " << key << " occurs as value and as subtree";
          throw std::range_error(message.str());
        }
        return true;
      }
      else
        return false;
  }


  /** \brief get value reference for key
   *
   * Returns reference to value for given key name.
   * This creates the key, if not existent.
   *
   * \param key key name
   * \return reference to corresponding value
   */
  std::string& operator[] (const std::string& key)
  {
    std::string::size_type dot = key.find(".");

    if (dot not_eq std::string::npos)
    {
      ConfigTree& s = sub(key.substr(0,dot));
      return s[key.substr(dot+1)];
    }
    else
    {
      if (not hasKey(key))
        valueKeys_.push_back(key);
      return values_[key];
    }
  }


  /** \brief get value reference for key
   *
   * Returns reference to value for given key name.
   * This creates the key, if not existent.
   *
   * \param key key name
   * \return reference to corresponding value
   * \throw Dune::RangeError if key is not found
   */
  const std::string& operator[] (const std::string& key) const
  {
    std::string::size_type dot = key.find(".");

    if (dot not_eq std::string::npos)
    {
      const ConfigTree& s = sub(key.substr(0,dot));
      return s[key.substr(dot+1)];
    }
    else
    {
      if (not hasKey(key))
      {
        std::ostringstream message;
        message << "Key '" << key << "' not found in ParameterTree (prefix " + prefix_ + ")";
        throw std::range_error(message.str());
      }
      return values_.find(key)->second;
    }
  }


  /** \brief print distinct substructure to stream
   *
   * Prints all entries with given prefix.
   *
   * \param stream Stream to print to
   * \param prefix for key and substructure names
   */
  void report(std::ostream& stream = std::cout,
              const std::string& prefix = "") const
  {
    typedef std::map<std::string, std::string>::const_iterator ValueIt;
    ValueIt vit = values_.begin();
    ValueIt vend = values_.end();

    for(; vit not_eq vend; ++vit)
      stream << vit->first << " = \"" << vit->second << "\"" << std::endl;

    typedef std::map<std::string, ConfigTree>::const_iterator SubIt;
    SubIt sit = subs_.begin();
    SubIt send = subs_.end();
    for(; sit not_eq send; ++sit)
    {
      stream << "[ " << prefix + prefix_ + sit->first << " ]" << std::endl;
      (sit->second).report(stream, prefix);
    }
  }


  /** \brief get substructure by name
   *
   * \param key substructure name
   * \return reference to substructure
   */
  ConfigTree& sub(const std::string& key)
  {
    std::string::size_type dot = key.find(".");

    if (dot not_eq std::string::npos)
    {
      ConfigTree& s = sub(key.substr(0,dot));
      return s.sub(key.substr(dot+1));
    }
    else
    {
      if (values_.count(key) > 0)
      {
        std::ostringstream message;
        message << "key " << key << " occurs as value and as subtree";
        throw std::range_error(message.str());
      }
      if (subs_.count(key) == 0)
        subKeys_.push_back(key.substr(0,dot));
      subs_[key].prefix_ = prefix_ + key + ".";
      return subs_[key];
    }
  }


  /** \brief get const substructure by name
   *
   * \param key              substructure name
   * \param fail_if_missing  if true, throw an error if substructure is missing
   * \return                 reference to substructure
   */
  const ConfigTree& sub(const std::string& key, bool fail_if_missing = false) const
  {
    std::string::size_type dot = key.find(".");

    if (dot not_eq std::string::npos)
    {
      const ConfigTree& s = sub(key.substr(0,dot));
      return s.sub(key.substr(dot+1));
    }
    else
    {
      if (values_.count(key) > 0)
      {
        std::ostringstream message;
        message << "key " << key << " occurs as value and as subtree";
        throw std::range_error(message.str());
      }
      if (subs_.count(key) == 0)
      {
        if (fail_if_missing)
        {
          std::ostringstream message;
          message << "SubTree '" << key << "' not found in ParameterTree (prefix " + prefix_ + ")";
          throw std::range_error(message.str());
        }
        else
          return empty_;
      }
      return subs_.find(key)->second;
    }
  }


  /** \brief get value as string
   *
   * Returns pure string value for given key.
   *
   * \param key key name
   * \param defaultValue default if key does not exist
   * \return value as string
   */
  std::string get(const std::string& key, const std::string& defaultValue) const
  {
    if (hasKey(key))
      return (*this)[key];
    else
      return defaultValue;
  }

  /** \brief get value as string
   *
   * Returns pure string value for given key.
   *
   * \todo This is a hack so get("my_key", "xyz") compiles
   * (without this method "xyz" resolves to bool instead of std::string)
   * \param key key name
   * \param defaultValue default if key does not exist
   * \return value as string
   */
  std::string get(const std::string& key, const char* defaultValue) const
  {
    if (hasKey(key))
      return (*this)[key];
    else
      return defaultValue;
  }


  /** \brief get value converted to a certain type
   *
   * Returns value as type T for given key.
   *
   * \tparam T type of returned value.
   * \param key key name
   * \param defaultValue default if key does not exist
   * \return value converted to T
   */
  template<typename T>
  T get(const std::string& key, const T& defaultValue) const
  {
    if(hasKey(key))
      return get<T>(key);
    else
      return defaultValue;
  }

  /** \brief Get value
   *
   * \tparam T Type of the value
   * \param key Key name
   * \throws RangeError if key does not exist
   * \throws NotImplemented Type is not supported
   * \return value as T
   */
  template <class T>
  T get(const std::string& key) const
  {
    if(not hasKey(key))
    {
      std::ostringstream message;
      message << "Key '" << key << "' not found in ParameterTree (prefix " + prefix_ + ")";
      throw std::range_error(message.str());
    }
    try
    {
      return Parser<T>::parse((*this)[key]);
    }
    catch(const std::range_error& e)
    {
      // rethrow the error and add more information
      std::ostringstream message;
      message << "Cannot parse value \"" << (*this)[key]
              << "\" for key \"" << prefix_ << "." << key << "\""
              << e.what();
      throw std::range_error(message.str());
    }
  }

  /** \brief get value keys
   *
   * Returns a vector of all keys associated to (key,values) entries in
   * order of appearance
   *
   * \return reference to entry vector
   */
  const KeyVector& getValueKeys() const
  {
    return valueKeys_;
  }


  /** \brief get substructure keys
   *
   * Returns a vector of all keys associated to (key,substructure) entries
   * in order of appearance
   *
   * \return reference to entry vector
   */
  const KeyVector& getSubKeys() const
  {
    return subKeys_;
  }

protected:

  static const ConfigTree empty_;

  std::string prefix_;

  KeyVector valueKeys_;
  KeyVector subKeys_;

  std::map<std::string, std::string> values_;
  std::map<std::string, ConfigTree> subs_;

  static std::string ltrim(const std::string& s)
  {
    std::size_t firstNonWS = s.find_first_not_of(" \t\n\r");

    if (firstNonWS not_eq std::string::npos)
      return s.substr(firstNonWS);
    return std::string();
  }
  static std::string rtrim(const std::string& s)
  {
    std::size_t lastNonWS = s.find_last_not_of(" \t\n\r");

    if (lastNonWS not_eq std::string::npos)
      return s.substr(0, lastNonWS+1);
    return std::string();
  }
  static std::vector<std::string> split(const std::string & s)
  {
    std::vector<std::string> substrings;
    std::size_t front = 0, back = 0, size = 0;

    while (front not_eq std::string::npos)
    {
      // find beginning of substring
      front = s.find_first_not_of(" \t\n\r", back);
      back  = s.find_first_of(" \t\n\r", front);
      size  = back - front;
      if (size > 0)
        substrings.push_back(s.substr(front, size));
    }
    return substrings;
  }

  // parse into a fixed-size range of iterators
  template<class Iterator>
  static void parseRange(const std::string &str,
                         Iterator it, const Iterator &end)
  {
    typedef typename std::iterator_traits<Iterator>::value_type Value;
    std::istringstream s(str);
    std::size_t n = 0;
    for(; it not_eq end; ++it, ++n)
    {
      s >> *it;
      if(not s)
      {
        std::ostringstream message;
        message << "as a range of items of type "
                << className<Value>()
                << " (" << n << " items were extracted successfully)";
        throw std::range_error(message.str());
      }
    }
    Value dummy;
    s >> dummy;
    // now extraction should have failed, and eof should be set
    if(not s.fail() or not s.eof())
      DUNE_THROW(RangeError, "as a range of "
        << n << " items of type "
        << className<Value>() << " (more items than the range can hold)");
  }
}; // end class ConfigTree

//==================================================================
// template specializations of struct Parser
// implement the parse method per type
//==================================================================

template<typename T>
struct ParameterTree::Parser
{
  static T parse(const std::string& str)
  {
    T val;
    std::istringstream s(str);
    // make sure we are in locale "C"
    s.imbue(std::locale::classic());
    s >> val;
    if(not s)
      DUNE_THROW(RangeError, " as a " << className<T>());
    T dummy;
    s >> dummy;
    // now extraction should have failed, and eof should be set
    if ((not s.fail()) or (not s.eof()))
      DUNE_THROW(RangeError, " as a " << className<T>());
    return val;
  }
};

// "How do I convert a string into a wstring in C++?"  "Why, that very simple
// son. You just need a these hundred lines of code."
// Instead im gonna restrict myself to string with charT=char here
template<typename traits, typename Allocator>
struct ParameterTree::Parser<std::basic_string<char, traits, Allocator> >
{
  static std::basic_string<char, traits, Allocator>
  parse(const std::string& str)
  {
    std::string trimmed = ltrim(rtrim(str));
    return std::basic_string<char, traits, Allocator>(trimmed.begin(),
                                                      trimmed.end());
  }
};

template<>
struct ParameterTree::Parser< bool >
{
  struct ToLower
  {
    char operator()(char c)
    {
      return std::tolower(c, std::locale::classic());
    }
  };

  static bool
  parse(const std::string& str)
  {
    std::string ret = str;

    std::transform(ret.begin(), ret.end(), ret.begin(), ToLower());

    if (ret == "yes" or ret == "true")
      return true;

    if (ret == "no" or ret == "false")
      return false;

    return (Parser<int>::parse(ret) not_eq 0);
  }
};

template<typename T, int n>
struct ParameterTree::Parser<FieldVector<T, n> >
{
  static FieldVector<T, n>
  parse(const std::string& str)
  {
    FieldVector<T, n> val;
    parseRange(str, val.begin(), val.end());
    return val;
  }
};

template<typename T, std::size_t n>
struct ParameterTree::Parser<std::array<T, n> >
{
  static std::array<T, n>
  parse(const std::string& str)
  {
    std::array<T, n> val;
    parseRange(str, val.begin(), val.end());
    return val;
  }
};

template<std::size_t n>
struct ParameterTree::Parser<std::bitset<n> >
{
  static std::bitset<n>
  parse(const std::string& str)
  {
    std::bitset<n> val;
    std::vector<std::string> sub = split(str);
    if (sub.size() not_eq n)
      DUNE_THROW(RangeError, "as a bitset<" << n << "> "
                 << "because of unmatching size " << sub.size());
    for (std::size_t i=0; i<n; ++i)
    {
      val[i] = ParameterTree::Parser<bool>::parse(sub[i]);
    }
    return val;
  }
};

template<typename T, typename A>
struct ParameterTree::Parser<std::vector<T, A> >
{
  static std::vector<T, A>
  parse(const std::string& str)
  {
    std::vector<std::string> sub = split(str);
    std::vector<T, A> vec;
    for (unsigned int i=0; i<sub.size(); ++i)
    {
      T val = ParameterTree::Parser<T>::parse(sub[i]);
      vec.push_back(val);
    }
    return vec;
  }
};

#endif
