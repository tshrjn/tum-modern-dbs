#include "operator/Register.hpp"

Register::Register(): type(Type::Undefined) {}
Register::~Register() {}

bool Register::operator==(const Register& r) const {
   // The type needs to be the same
   if (type!=r.type) {
      return false;
   }
   // Compare the state based on the type
   switch (type) {
      case Type::Undefined: return true;
      case Type::Int: return intValue==r.intValue;
      case Type::Double: return doubleValue==r.doubleValue;
      case Type::Bool: return boolValue==r.boolValue;
      case Type::String: return stringValue==r.stringValue;
   }
   return false;
}


bool Register::operator<(const Register& r) const {
   if (type!=r.type) {
      return static_cast<unsigned>(type)<static_cast<unsigned>(r.type);
   }
   switch (type) {
      case Type::Undefined: return false;
      case Type::Int: return intValue<r.intValue;
      case Type::Double: return doubleValue<r.doubleValue;
      case Type::Bool: return boolValue<r.boolValue;
      case Type::String: return stringValue<r.stringValue;
   }
   return false;
}


static uint64_t computeHash(const void* buffer,unsigned len) {
   static const uint64_t FNV_offset_basis=14695981039346656037ull;
   static const uint64_t FNV_prime=1099511628211ull;

   uint64_t hash=FNV_offset_basis;
   for (const uint8_t* iter=static_cast<const uint8_t*>(buffer),*limit=iter+len;iter!=limit;++iter)
      hash=(hash^(*iter))*FNV_prime;
   return hash;
}


unsigned Register::getHash() const {
   switch (type) {
      case Type::Undefined: return 0;
      case Type::Int: return intValue;
      case Type::Double: return computeHash(&doubleValue,sizeof(doubleValue));
      case Type::Bool: return boolValue;
      case Type::String: return computeHash(stringValue.data(),stringValue.length());
   }
   return 0;
}