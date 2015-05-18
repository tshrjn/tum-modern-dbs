#ifndef H_Schema_hpp
#define H_Schema_hpp

#include <vector>
#include <string>
#include "schema/Types.cpp"

#include <iostream>
#include <string>
#include <memory>

struct Schema {
   struct Relation {
      struct Attribute {
         std::string name;
         Types::Tag type;
         unsigned len;
         bool notNull;
         Attribute() : len(~0), notNull(true) {}
      };
      
      // The "head" of a relation contains the relation name, the segment_id and number_pages
      std::string name;
      uint16_t segmentId;
      uint64_t numberPages;

      // Of course a relation contains multiple attributes
      std::vector<Schema::Relation::Attribute> attributes;

      // And a primary key
      std::vector<unsigned> primaryKey;

      Relation(const std::string& name,
         const uint16_t& segmentId, const uint64_t& numberPages)
         : name(name), segmentId(segmentId), numberPages(numberPages) {
            std::cout << "segmentId: " << segmentId << std::endl;
            std::cout << "numberPages: " << numberPages << std::endl;
         }
   };
   std::vector<Schema::Relation> relations;
   std::string toString() const;

public:
   std::string serialize() const;
   static std::unique_ptr<Schema> deserialize(const char *data);
};
#endif
