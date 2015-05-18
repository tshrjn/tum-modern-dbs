#include "Schema.hpp"

#include <sstream>

static std::string type(const Schema::Relation::Attribute& attr) {
   Types::Tag type = attr.type;
   switch(type) {
      case Types::Tag::Integer:
         return "Integer";
      /*case Types::Tag::Numeric: {
         std::stringstream ss;
         ss << "Numeric(" << attr.len1 << ", " << attr.len2 << ")";
         return ss.str();
      }*/
      case Types::Tag::Char: {
         std::stringstream ss;
         ss << "Char(" << attr.len << ")";
         return ss.str();
      }
   }
   throw;
}

std::string Schema::toString() const {
   std::stringstream out; 
   for (const Schema::Relation& rel : relations) {
      out << rel.name << " (" << rel.segmentId << ") " << rel.numberPages << " pages" << std::endl;
      out << "\tPrimary Key:";
      for (unsigned keyId : rel.primaryKey)
         out << ' ' << rel.attributes[keyId].name;
      out << std::endl;
      for (const auto& attr : rel.attributes)
         out << '\t' << attr.name << '\t' << type(attr) << (attr.notNull ? " not null" : "") << std::endl;
   }
   return out.str();
}

std::string Schema::serialize() const {
   std::cout << "Schema.serialize" << std::endl;
 
   std::stringstream out;

    // Transform schema object to an easily readable string.
    out << relations.size() << std::endl;
    for (const Schema::Relation& rel : relations) {
        // header
        out << rel.name << " " << rel.segmentId << " " << rel.numberPages << std::endl;
        // length of the arrays
        out << rel.attributes.size() << " " << rel.primaryKey.size() << std::endl;

        for (unsigned keyId : rel.primaryKey)
            out << keyId << " ";
        out << std::endl;
        for (const auto& attr : rel.attributes) {
            out << attr.name << " ";
            if (attr.type == Types::Tag::Integer) {
                out << "0";
            } else {
                out << "1";
            }
            out << " " << attr.len << " " << (attr.notNull ? "1" : "0") << std::endl;
        }
    }

    // Store the string of the schema in the data pointer of a buffer frame.
    std::string schemaString = out.str();

    std::cout << "Schema.serialize: Successfully serialzed schema" << std::endl; 
    std::cout << schemaString << std::endl;

    return schemaString;
}

std::unique_ptr<Schema> Schema::deserialize(const char *data) {
   std::cout << "Schema.deserialize" << std::endl;

   auto schema = std::unique_ptr<Schema>(new Schema());

   std::stringstream ss;
   ss << data;

   // Build a new schema from scratch
   int relations;
   ss >> relations;
   // parse relations
   for (int i = 0; i < relations; i++) {
      std::string name;
      uint16_t segmentId;
      uint64_t pages;
      int attributes, keys;

      // header
      ss >> name >> segmentId >> pages;

      // length of arrays
      ss >> attributes >> keys;

      Schema::Relation relation(name, segmentId, pages);

      // parse keys
      for (int j = 0; j < keys; j++) {
         unsigned key;
         ss >> key;
         relation.primaryKey.push_back(key);
      }

      // parse attributes
      for (int j = 0; j < attributes; j++) {
         std::string name;
         unsigned type;
         unsigned len;
         bool notNull;

         ss >> name >> type >> len >> notNull;

         Schema::Relation::Attribute attribute;
         attribute.name = name;
         if (type == 0) {
            attribute.type = Types::Tag::Integer;
         } else {
            attribute.type = Types::Tag::Char;
         }
         attribute.len = len;
         attribute.notNull = notNull;

         relation.attributes.push_back(attribute);
      }
      schema->relations.push_back(relation);
   }

   std::cout << "Schema.deserialize: Successfully deserialized schema" << std::endl; 
   std::cout << schema->toString() << std::endl;

   return move(schema);
}
