#ifndef H_Parser_hpp
#define H_Parser_hpp

#include <exception>
#include <string>
#include <memory>
#include <fstream>
#include <sstream>
#include "schema/Schema.hpp"
#include <sstream>

class ParserError : std::exception {
   std::string msg;
   unsigned line;
   public:
   ParserError(unsigned line, const std::string& m) : msg(m), line(line) {}
   ~ParserError() throw() {}
   const char* what() const throw() {
      std::stringstream ss;
      ss << line << ": ";
      ss << msg;
      return ss.str().c_str();
   }
};

struct Parser {
   std::string fileName;
   std::ifstream in;
   enum class State : unsigned { Init, Create, Table, CreateTableBegin, CreateTableEnd, TableName, Primary, Key, KeyListBegin, KeyName, KeyListEnd, AttributeName, AttributeTypeInt, AttributeTypeChar, CharBegin, CharValue, CharEnd, AttributeTypeNumeric, NumericBegin, NumericValue1, NumericSeparator, NumericValue2, NumericEnd, Not, Null, Separator, Semicolon};
   State state;
   Parser(const std::string& fileName) : fileName(fileName), state(State::Init) {}
   ~Parser() {};
   std::unique_ptr<Schema> parse();

   private:
   void nextToken(unsigned line, const std::string& token, Schema& s);

   uint16_t nextSegmentId = 1;
};

#endif
