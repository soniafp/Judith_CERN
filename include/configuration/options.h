#ifndef OPTIONS_H
#define OPTIONS_H

#include <string>
#include <map>
#include <set>
#include <vector>
#include <sstream>

namespace Configuration {

/**
  * Parses command line and setting file data into argument / values pairs.
  * Provides methods to access values for a given argument.
  *
  * At the core of the object is a `std::map` which maps arguments
  * (`std::string`) to values (`std::vector<std::string>`). The map is filled
  * from command line arguments, from a text file, or manually. Values are
  * appended to the same argument when the argument appears multiple times
  * except when parsing command line arguments.
  *
  * Example:
  * Options& options = Options::getInstance();
  * options.defineShort('c', "argname");
  * options.parseArgs(argc, argv);
  * options.parseFile(pathToSettings);
  * options.addPair("argname", "default value");
  * const std::string& val = options.getValue("argname");
  *
  * `val` will then contain the first value after 'c' or "argname" from the
  * command line arguments, or the first value after "argname" in the settings
  * file, or the provided "default value" (in that order of priority).
  *
  * @author Garrin McGoldrick (garrin.mcgoldrick@cern.ch)
  */
class Options {
public:
  /** Type for values corresponding from a key/value pair. This will type must
    * have random access. */
  typedef std::vector<std::string> Values;
  /** Type for the key/value pair maps */
  typedef std::map<std::string, Values> Pairs;

private:
  /** Map of arguments to their values */
  Pairs m_pairs;

  /** Remembers parsed files to avoid re-processing a file */
  std::set<std::string> m_parsedPaths;
  /** Map for command line shorthands to full arguments */
  std::map<char, std::string> m_mapShort;

  /** Used when need to pass a refernece to a non-existing string */
  const std::string m_dummyStr;
  /** Used when need to pass a refernece to a non-existing values */
  const Values m_dummyVec;

  void appendValues(const std::string& arg, const Values& vals);

public:
  /** Default constructor */
  Options() {}
  /** Default destructor */
  ~Options() {}

  /** Maps each argument -x or --xx from `argv` to a list of space delimited
    * values following the argument (preceeding the next argument). The mapped
    * pairs are stored in m_pairs. Throws an error for duplicate arguments */
  void parseArgs(int argc, const char** argv);

  /** Maps each argument (first word of a line) from the file at `filePath` to
    * a list of space delimited strings following the setting. Ignores anything
    * following a # character. Keeps appending values for the same arg. */
  void parseFile(const std::string& filePath);

  /** Get a constant reference to the argument/value pair map */
  const Pairs& pairs() const { return m_pairs; }

  /** Generate a vector of strings from all args. Generated a new vector at
    * each call since the m_paris contents could change. Return type is same
    * as Values, but isn't actually values, so explicitely type it */
  std::vector<std::string> listArgs() const;

  /** Get the first value for an argument. If the argument has no values
    * or doesn't exist, returns and empty string */
  const std::string& getValue(const std::string& arg) const;

  /** Get the vector of values for an argument. If the argument doesn't exist
    * returns an empty Values vector. */
  const Values& getValues(const std::string& arg) const;

  /** Check whether or not an argument is in the map */
  bool hasArg(const std::string& arg) const;

  /** Evaluate the boolean outcome of an argument */
  bool evalBoolArg(const std::string& arg) const;

  /** Manually insert an argument and values. The string will be parsed into
    * space separated values. */
  void addPair(const std::string& arg, const std::string& val);

  /** Map a command line short argument to its full name */
  void defineShort(char c, const std::string& arg);

  /** Fills the variable `val` with the contents of the string `str` */
  template <class T>
  static inline void strToVal(const std::string& str, T& val) {
    std::stringstream ss(str);
    ss >> val;
  }

  /** Returns a double value representation of `str` */
  inline double strToFloat(const std::string& str) {
    double val = 0;
    strToVal(str, val);
    return val;
  }

  /** Returns an integer value representation of `str` */
  inline int strToInt(const std::string& str) {
    int val = 0;
    strToVal(str, val);
    return val;
  }

  /** Returns true if the string is true, on, 1 or if it is empty as is the
    * case when no value is provided for an argument. */
  static inline bool strToBool(const std::string& str) {
    if (str == "true") return true;
    if (str == "on") return true;
    if (str == "1") return true;
    if (str.empty()) return true;
    return false;
  }
};

}

#endif  // OPTIONS_H
