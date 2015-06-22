#ifndef HashJoin_HPP
#define HashJoin_HPP

#include "operator/Operator.hpp"
#include "operator/Register.hpp"
#include <memory>
#include <unordered_map>


class HashJoin : public Operator
{
   private:
   // Recurive operators that can be used to pull values
   std::unique_ptr<Operator> left,right;

   // Register that hold the left and right attribute to join at
   // With next() a new value is loaded into these registers
   const Register* leftValue,*rightValue;

   // Left Registers
   std::vector<Register*> leftRegs;

   // Hashtable
   std::unordered_multimap<Register,std::vector<Register>,Register::hash> table;
   // Hash iterators
   std::unordered_multimap<Register,std::vector<Register>,Register::hash>::const_iterator iter, limit;

   public:
   HashJoin(
      std::unique_ptr<Operator>&& left,
      std::unique_ptr<Operator>&& right,
      const Register* leftValue,
      const Register* rightValue);
   ~HashJoin();

   void open();
   bool next();
   void close();
   std::vector<const Register*> getOutput() const;
};
#endif
