#ifndef REGISTER_HPP
#define REGISTER_HPP

#include <string>

class Register
{
   public:

   enum class Type : unsigned { Int, Double, Bool, String, Undefined };

   private:

   // The type of a given Register
   Type type;

   union { int intValue; double doubleValue; bool boolValue; };
   std::string stringValue;

   public:
   Register();
   ~Register();

   // Setters
   void setUnbound() { type=Type::Undefined; }
   void setInt(int value) { type=Type::Int; intValue=value; }
   void setDouble(double value) { type=Type::Double; doubleValue=value; }
   void setBool(bool value) { type=Type::Bool; boolValue=value; }
   void setString(const std::string& value) { type=Type::String; stringValue=value; }

   // Indicator if the Register type is defined or not
   bool isUndefined() const { return type==Type::Undefined; }

   // Getters
   Type getType() const { return type; }
   int getInt() const { return intValue; }
   double getDouble() const { return doubleValue; }
   bool getBool() const { return boolValue; }
   const std::string& getString() const { return stringValue; }

   // Comparison
   bool operator==(const Register& r) const;
   bool operator<(const Register& r) const;

   // Hashable
   unsigned getHash() const;
   struct hash { unsigned operator()(const Register& r) const { return r.getHash(); }; };
};
#endif
