#ifndef OPERATOR_HPP
#define OPERATOR_HPP

#include <vector>

class Register;

// Base class for all operators
class Operator
{
   public:
   Operator();
   virtual ~Operator();
   virtual void open() = 0;
   virtual bool next() = 0;
   virtual std::vector<const Register*> getOutput() const = 0;
   virtual void close() = 0;
};
#endif
