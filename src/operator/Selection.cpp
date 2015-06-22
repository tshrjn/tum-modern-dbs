#include "operator/Selection.hpp"
#include "operator/Register.hpp"

using namespace std;

Selection::Selection(
   unique_ptr<Operator>&& input,
   const Register* a,
   const Register* b)
   : input(move(input)),
      condition(a),
      equal(b) {
}

Selection::~Selection() {}


void Selection::open() {
   input->open();
}


bool Selection::next() {
   while (true) {
      // Produce a tuple
      if (!input->next())
         return false;
      // Check
      if (equal) {
         if (condition->getState()==equal->getState()) switch (condition->getState()) {
            case Register::State::Unbound: break;
            case Register::State::Int: if (condition->getInt()==equal->getInt()) return true; break;
            case Register::State::Double: if (condition->getDouble()==equal->getDouble()) return true; break;
            case Register::State::Bool: if (condition->getBool()==equal->getBool()) return true; break;
            case Register::State::String: if (condition->getString()==equal->getString()) return true; break;
         }
      } else {
         if ((condition->getState()==Register::State::Bool)&&(condition->getBool()))
            return true;
      }
   }
}

void Selection::close() {
   input->close();
}

vector<const Register*> Selection::getOutput() const {
   return input->getOutput();
}