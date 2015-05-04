#include <iostream>
#include <fstream>
#include <cassert>
#include <map>
#include <set>
#include <list>
#include <vector>
#include <cctype>

#include "options.h"

void Options::appendValues(const std::string& arg, const Values& vals) {
  // Find this setting in the map of settings
  Pairs::iterator it = m_pairs.find(arg);

  // If the setting is found, append values to those existing
  if (it != m_pairs.end()) {
    Values& current = it->second;
    current.insert(current.end(), vals.begin(), vals.end());
  }
  // Otherwise create a new setting entry and set its values
  else {
    m_pairs[arg] = vals;
  }
}

void Options::parseArgs(int argc, const char** argv) {
  std::string arg;  // last argument
  std::list<std::string> valList; // values accumualted for last arg

  // Loop over all white space delimited words from command line. Make one
  // additional iteration to save the last stored option
  for (int i = 0; i < argc+1; i++) {
    // Store the word unless this is the last iteration
    const std::string tmp = (i < argc) ? argv[i] : "";
    
    // Identify words of the form -c or --string. `c` is a single alphabetic
    // character

    // If this is the last iteration, or if this new word is an argument, then
    // store the values collected for the last argument
    if ((i == argc) ||
        (tmp.size() == 2 && tmp[0] == '-' && isalpha(tmp[1])) ||
        (tmp.size() > 2 && tmp.substr(0, 2) == "--")) {
      // Make sure there is a prior argument to store
      if (!arg.empty()) { 
        // If this is a short argument of the form -c
        if (arg.size() == 2) {
          arg = arg.substr(1);  // Remove the -
          // See if it maps to a full argument
          std::map<char, std::string>::iterator it;
          if ((it = m_mapShort.find(arg[0])) != m_mapShort.end())
            arg = it->second;  // Get its full argument
        }
        // Otherwise it is a long argument of the form --string
        else {
          arg = arg.substr(2);  // Remove the --
        }

        // Ensure the option isn't already in the map
        if (m_pairs.find(arg) != m_pairs.end())
          throw "Options::parseArgs: Duplicate argument detected";

        // Map values collected to this argument. Make a vector from the list.
        m_pairs[arg] = Values(valList.begin(), valList.end());
      }
      arg = tmp;  // Store the new option
      valList.clear();  // Clear values from last option
    }

    // The word isn't an option, store it as a value
    else if (!tmp.empty()) {
      valList.push_back(tmp);
    }
  }
}

void fillSpaceDelimited(
    const std::string& parse,
    std::vector<std::string>& fill) {
  // Fill a temporary list with each space delimited word. Lists could be very
  // long, so best to use a list container.
  std::list<std::string> tmp;
  std::string word;
  std::stringstream ss(parse);
  while (std::getline(ss, word, ' ')) {
    if (word.empty()) continue;  // padded spaces
    if (word[0] == '#') break;  // stop parsing at comment
    tmp.push_back(word);
  }
  // Turn into a vector for random access by user
  fill.assign(tmp.begin(), tmp.end());
}

void Options::parseFile(const std::string& filePath) {
  // Don't re-parse a file
  if (m_parsedPaths.find(filePath) != m_parsedPaths.end()) return;
  m_parsedPaths.insert(filePath);

  std::ifstream file(filePath.c_str());
  if (!file.is_open())
    throw "Options::praseFile: unable to open file";

  std::string line;

  // Read each line in the file
  while (std::getline(file, line)) {
    // Skip blank lines and comment lines
    const size_t start = line.find_first_not_of(" \t\r\n");
    if (start == std::string::npos || line.substr(start)[0] == '#')
      continue;

    // Get each space separated field into a vector
    Values fields;
    fillSpaceDelimited(line.substr(start), fields);

    if (fields.empty()) continue;

    // The first field is the setting argument, the rest are its values
    const std::string arg = fields[0];
    fields.erase(fields.begin());

    appendValues(arg, fields);
  }
}

/** Fill a vector with key values from `map` */
void listKeys(
    std::vector<std::string>& fill,
    const Options::Pairs& map) {
  // Prepare the vector to be filled
  fill.clear(); 
  fill.reserve(map.size());
  // Iterate through the mapped pairs
  for (Options::Pairs::const_iterator it = map.begin();
      it != map.end(); ++it) {
    fill.push_back(it->first);  // Append map key
  }
}

std::vector<std::string> Options::listArgs() const {
  std::vector<std::string> args;
  listKeys(args, m_pairs);
  return args;
}

const std::string& Options::getValue(
    const std::string& arg) const {
  Pairs::const_iterator it = m_pairs.find(arg);
  // If there is no arg, or it has no values, return "", otherwise return the
  // first value from its values
  return it == m_pairs.end() || it->second.empty() ? m_dummyStr : it->second[0];
}

const std::vector<std::string>& Options::getValues(
    const std::string& arg) const {
  Pairs::const_iterator it = m_pairs.find(arg);
  // If there is no arg, return an empty vector, otherwise return the values
  return it == m_pairs.end() ? m_dummyVec : it->second;
}

bool Options::hasArg(const std::string& arg) const {
  return m_pairs.find(arg) != m_pairs.end();
}

bool Options::evalBoolArg(const std::string& arg) const {
  // If a key exists, and its value evaluates to true (also if it has no value)
  // then resturns true. If no key or evalutes to false, returns false.
  return hasArg(arg) && strToBool(getValue(arg));
}

void Options::addPair(const std::string& arg, const std::string& val) {
  // Make a values vector from the given string
  Values vals;
  fillSpaceDelimited(val, vals);
  // Add to the pair map
  appendValues(arg, vals);
}

void Options::defineShort(char c, const std::string& arg) {
  // Check if this short argument is alredy defined
  if (m_mapShort.find(c) != m_mapShort.end())
    throw "Options::defineShort: duplicate short argument";
  // Map this short argument to a full argument name
  m_mapShort[c] = arg;
}

