// -*- tab-width: 2; indent-tabs-mode: nil -*-
// vi: set et ts=2 sw=2 sts=2:
#ifndef CONFIGTREE_PARSER_HH
#define CONFIGTREE_PARSER_HH

/** \file
 * \brief Various parser methods to get data into a ConfigTree object
 */

#include <istream>
#include <string>
#include <vector>
#include <set>

#include "configtree.hh"

class ConfigTreeParser
{

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

public :

  /** @name Parsing methods for the INITree file format
   *
   *  INITree files should look like this
   *  \verbatim
   * # this file configures fruit colors in fruitsalad
   *
   *
   * #these are no fruit but could also appear in fruit salad
   * honeydewmelon = yellow
   * watermelon = green
   *
   * fruit.tropicalfruit.orange = orange
   *
   * [fruit]
   * strawberry = red
   * pomegranate = red
   *
   * [fruit.pipfruit]
   * apple = green/red/yellow
   * pear = green
   *
   * [fruit.stonefruit]
   * cherry = red
   * plum = purple
   *
   * \endverbatim
   *
   *
   * If a '[prefix]' statement appears all following entries use this prefix
   * until the next '[prefix]' statement. Fruitsalads for example contain:
   * \verbatim
   * honeydewmelon = yellow
   * fruit.tropicalfruit.orange = orange
   * fruit.pipfruit.apple = green/red/yellow
   * fruit.stonefruit.cherry = red
   * \endverbatim
   *
   * All keys with a common 'prefix.' belong to the same substructure called
   * 'prefix'.  Leading and trailing spaces and tabs are removed from the
   * values unless you use single or double quotes around them.  Using single
   * or double quotes you can also have multiline values.
   */
  //@{

  /** \brief parse C++ stream
   *
   * Parses C++ stream and build hierarchical config structure.
   *
   * \param in        The stream to parse
   * \param[out] pt        The parameter tree to store the config structure.
   * \param overwrite Whether to overwrite already existing values.
   *                  If false, values in the stream will be ignored
   *                  if the key is already present.
   *
   * \note This method is identical to parseStream(std::istream&,
   *       const std::string&, bool) with the exception that that
   *       method allows one to give a custom name for the stream.
   */
  static void readINITree(std::istream& in, ConfigTree& pt,
                          bool overwrite)
  {
    readINITree(in, pt, "stream", overwrite);
  }


  /** \brief parse C++ stream
   *
   * Parses C++ stream and build hierarchical config structure.
   *
   * \param in      The stream to parse
   * \param[out] pt      The parameter tree to store the config structure.
   * \param srcname Name of the configuration source for error
   *                messages, "stdin" or a filename.
   * \param overwrite Whether to overwrite already existing values.
   *                  If false, values in the stream will be ignored
   *                  if the key is already present.
   */
  static void readINITree(std::istream& in, ConfigTree& pt,
                          const std::string srcname = "stream",
                          bool overwrite = true)
  {
    std::string prefix;
    std::set<std::string> keysInFile;
    while(not in.eof())
    {
      std::string line;
      getline(in, line);
      line = ltrim(line);
      if (line.size() == 0)
        continue;
      switch (line[0]) {
      case '#' :
        break;
      case '[' :
        line = rtrim(line);
        if (line[line.length()-1] == ']')
        {
          prefix = rtrim(ltrim(line.substr(1, line.length()-2)));
          if (prefix != "")
            prefix += ".";
        }
        break;
      default :
        std::string::size_type comment = line.find("#");
        line = line.substr(0,comment);
        std::string::size_type mid = line.find("=");
        if (mid not_eq std::string::npos)
        {
          std::string key = prefix+rtrim(ltrim(line.substr(0, mid)));
          std::string value = ltrim(line.substr(mid+1));

          if (value.length()>0)
          {
            // handle quoted strings
            if ((value[0]=='\'') or (value[0]=='"'))
            {
              char quote = value[0];
              value=value.substr(1);
              while (*(rtrim(value).rbegin()) not_eq quote)
              {
                if (not in.eof())
                {
                  std::string l;
                  getline(in, l);
                  value = value+"\n"+l;
                }
                else
                  value = value+quote;
              }
              value = rtrim(value);
              value = value.substr(0,value.length()-1);
            }
            else
              value = rtrim(value);
          }

          if (keysInFile.count(key) not_eq 0)
          {
            std::ostringstream message;
            message << "Key '" << key << "' appears twice in " << srcname << " !";
            throw std::range_error(message.str());
          }
          else
          {
            if(overwrite or not pt.hasKey(key))
              pt[key] = value;
            keysInFile.insert(key);
          }
        }
        break;
      }
    }

  }


  /** \brief parse file
   *
   * Parses file with given name and build hierarchical config structure.
   *
   * \param file filename
   * \param[out] pt   The parameter tree to store the config structure.
   * \param overwrite Whether to overwrite already existing values.
   *                  If false, values in the stream will be ignored
   *                  if the key is already present.
   */
  static void readINITree(std::string file, ConfigTree& pt, bool overwrite = true)
  {
    std::ifstream in(file.c_str());

    if (not in)
    {
      std::ostringstream message;
      message << "Could not open configuration file " << file;
      throw std::ifstream::failure(message.str());
    }

    readINITree(in, pt, "file '" + file + "'", overwrite);
  }

  //@}

  /** \brief parse command line options and build hierarchical ConfigTree structure
   *
   * The list of command line options is searched for pairs of the type <kbd>-key value</kbd>
   * (note the hyphen in front of the key).
   * For each such pair of options a key-value pair with the corresponding names
   * is then created in the ConfigTree.
   *
   * \param argc arg count
   * \param argv arg values
   * \param[out] pt   The parameter tree to store the config structure.
   */
  static void readOptions(int argc, char* argv [], ConfigTree& pt)
  {
    for(int i=1; i<argc; i++)
    {
      if ((argv[i][0]=='-') and (argv[i][1] not_eq '\000'))
      {
        if(argv[i+1] == nullptr)
        {
          std::ostringstream message;
          message << "last option on command line (" << argv[i]
                  << ") does not have an argument";
          throw std::range_error(message.str());
        }
        pt[argv[i]+1] = argv[i+1];
        ++i; // skip over option argument
      }
    }
  }

  /**
   * \brief read [named] command line options and build hierarchical ConfigTree structure
   *
   * Similar to pythons named options we expect the parameters in the
   * ordering induced by keywords, but allow the user to pass named options
   * in the form of --key=value. Optionally the user can pass an additional
   * vector with help strings.
   *
   * \param argc arg count
   * \param argv arg values
   * \param[out] pt   The parameter tree to store the config structure.
   * \param keywords vector with keywords names
   * \param required number of required options (the first n keywords are required, default is all are required)
   * \param allow_more allow more options than these listed in keywords (default = true)
   * \param overwrite  allow to overwrite existing options (default = true)
   * \param help vector containing help strings
  */
  static void readNamedOptions(int argc, char* argv[],
                               ConfigTree& pt,
                               std::vector<std::string> keywords,
                               unsigned int required = std::numeric_limits<unsigned int>::max(),
                               bool allow_more = true,
                               bool overwrite = true,
                               std::vector<std::string> help = std::vector<std::string>())
  {
    std::string helpstr = generateHelpString(argv[0], keywords, required, help);
    std::vector<bool> done(keywords.size(),false);
    std::size_t current = 0;

    for (std::size_t i=1; i<std::size_t(argc); i++)
    {
      std::string opt = argv[i];
      // check for help
      if (opt == "-h" or opt == "--help")
        throw std::invalid_argument(helpstr);
      // is this a named parameter?
      if (opt.substr(0,2) == "--")
      {
        size_t pos = opt.find('=',2);
        if (pos == std::string::npos)
        {
          std::ostringstream message;
          message << "value missing for parameter " << opt << "\n" << helpstr;
          throw std::range_error(message.str());
        }
        std::string key = opt.substr(2,pos-2);
        std::string value = opt.substr(pos+1,opt.size()-pos-1);
        auto it = std::find(keywords.begin(), keywords.end(), key);
        // is this param in the keywords?
        if (not allow_more and it == keywords.end())
        {
          std::ostringstream message;
          message << "unknown parameter " << key << "\n" << helpstr;
          throw std::range_error(message.str());
        }
        // do we overwrite an existing entry?
        if (not overwrite and pt[key] not_eq "")
        {
          std::ostringstream message;
          message << "parameter " << key << " already specified" << "\n" << helpstr;
          throw std::range_error(message.str());
        }
        pt[key] = value;
        if(it not_eq keywords.end())
          done[std::distance(keywords.begin(),it)] = true; // mark key as stored
      }
      else {
        // map to the next keyword in the list
      while(current < done.size() and done[current]) ++current;
      // are there keywords left?
      if (current >= done.size())
      {
        std::ostringstream message;
        message << "superfluous unnamed parameter" << "\n" << helpstr;
        throw std::range_error(message.str());
      }
      // do we overwrite an existing entry?
      if (not overwrite and pt[keywords[current]] not_eq "")
      {
        std::ostringstream message;
        message << "parameter " << keywords[current] << " already specified" << "\n" << helpstr;
        throw std::range_error(message.str());
      }
      pt[keywords[current]] = opt;
      done[current] = true; // mark key as stored
      }
    }
    // check that we receive all required keywords
    std::string missing = "";
    for (unsigned int i=0; i<keywords.size(); i++)
      if ((i < required) and not done[i]) // is this param required?
        missing += std::string(" ") + keywords[i];
    if (missing.size())
    {
      std::ostringstream message;
      message << "missing parameter(s) ... " << missing << "\n" << helpstr;
      throw std::range_error(message.str());
    }
  }

private:
  static std::string generateHelpString(std::string progname, std::vector<std::string> keywords, unsigned int required, std::vector<std::string> help)
  {
    static const char braces[] = "<>[]";
    std::string helpstr = "";
    helpstr = helpstr + "Usage: " + progname;
    for (std::size_t i=0; i<keywords.size(); i++)
    {
      bool req = (i < required);
      helpstr = helpstr +
        " " + braces[req*2] +
        keywords[i] +braces[req*2+1];
    }
    helpstr = helpstr + "\n"
      "Options:\n"
      "-h / --help: this help\n";
    for (std::size_t i=0; i<std::min(keywords.size(),help.size()); i++)
    {
      if (help[i] not_eq "")
        helpstr = helpstr + "-" +
          keywords[i] + ":\t" + help[i] + "\n";
    }
    return helpstr;
  }
}; // end class ConfigTreeParser

#endif
