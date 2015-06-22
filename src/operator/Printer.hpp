#ifndef H_operator_Printer
#define H_operator_Printer

#include "operator/Operator.hpp"
#include <memory>

/// Prints tuple attributes
class Printer : public Operator {
   private:
   std::unique_ptr<Operator> input;
   std::vector<const Register*> toPrint;

   public:
   explicit Printer(std::unique_ptr<Operator>&& input);
   Printer(std::unique_ptr<Operator>&& input,const std::vector<const Register*>& toPrint);

   void open();
   bool next();
   void close();

   std::vector<const Register*> getOutput() const;
};
#endif
