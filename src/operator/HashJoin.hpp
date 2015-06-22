#ifndef HashJoin_HPP
#define HashJoin_HPP

#include "operator/Operator.hpp"
#include "operator/Register.hpp"
#include <memory>
#include <unordered_map>


class HashJoin : public Operator
{
   private:
   std::unique_ptr<Operator> left,right;
   const Register* leftValue,*rightValue;
   std::vector<Register*> leftRegs;


   std::unordered_multimap<Register,std::vector<Register>,Register::hash> table;
   std::unordered_multimap<Register,std::vector<Register>,Register::hash>::const_iterator iter,iterLimit;

   public:
   HashJoin(std::unique_ptr<Operator>&& left,std::unique_ptr<Operator>&& right,const Register* leftValue,const Register* rightValue);
   ~HashJoin();

   void open();
   bool next();
   void close();

   std::vector<const Register*> getOutput() const;
};
#endif
