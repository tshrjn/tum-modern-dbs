#ifndef H_Schema_hpp
#define H_Schema_hpp

#include <vector>
#include <string>
#include "schema/Types.cpp"

#include <iostream>
#include <string>

struct Schema {
   struct Relation {
      struct Attribute {
         std::string name;
         Types::Tag type;
         unsigned len;
         bool notNull;
         Attribute() : len(~0), notNull(true) {}
      };
      std::string name;
      std::vector<Schema::Relation::Attribute> attributes;
      std::vector<unsigned> primaryKey;
      Relation(const std::string& name) : name(name) {}
   };
   std::vector<Schema::Relation> relations;
   std::string toString() const;

public:
   std::string serialize() const;
   static std::unique_ptr<Schema> deserialize(const char *data);
};
#endif
