#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include <string>
#include <sstream>
#include <fstream>
#include <vector>

class ConfigParser
{
public:
  struct Row {
    std::string header;
    std::string key;
    std::string value;
    bool        isHeader;
  };

private:
  const char*   _filePath;
  std::ifstream _inputFile;
  std::string   _lineBuffer;
  std::string   _currentHeader;
  std::string   _currentKey;
  std::string   _currentValue;

  unsigned int _numRows;
  std::vector<Row> _parsedContents;

  bool checkRange(size_t start, size_t end);

  // These functions return -1 if they can't find the desired element
  int readNextLine(std::ifstream& input); // Fill line buffer with next line
  int parseForHeader(); // Parse the line buffer for a header
  int parseForKey(); // Parse the line buffer for a key
  int parseForLink(); // Parse the line for a link to another config
  void parseContents(std::ifstream& input); // Parse the entire file's contents

public:
  ConfigParser(const char* filePath);

  void print();

  unsigned int getNumRows() const;
  const Row* getRow(unsigned int n) const;
  std::vector<Row> getParsedConents() const;
  const char* getFilePath() const;

  static double valueToNumerical(const std::string& value);
  static bool   valueToLogical(const std::string& value);
  static void   valueToVec(const std::string& value, std::vector<double>& vec);

private:
  ConfigParser(const ConfigParser&); // Disable the copy constructor
  ConfigParser& operator=(const ConfigParser&); // Disable the assignment operator
};

#endif // CONFIGPARSER_H
