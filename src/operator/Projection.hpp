#ifndef H_operator_Projection
#define H_operator_Projection

#include "operator/Operator.hpp"
#include <memory>

class Projection : public Operator
{
   private:
   std::unique_ptr<Operator> input;
   std::vector<uint> registerIdzs;

   public:
   Projection(
      std::unique_ptr<Operator>&& input,
      const std::vector<uint>& registerIdzs);
   ~Projection();

   void open();
   bool next();
   void close();

   std::vector<const Register*> getOutput() const;
};
#endif
