#include "operator/Selection.hpp"
#include "operator/Register.hpp"

using namespace std;

Selection::Selection(
        unique_ptr < Operator > && input,
const
Register *a,
const Register *b
)
:

input(move(input)),
condition(a),
equal(b) {
}

Selection::~Selection() { }


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
            if (condition->getType() == equal->getType())
                switch (condition->getType()) {
                    case Register::Type::Undefined:
                        break;
                    case Register::Type::Int:
                        if (condition->getInt() == equal->getInt()) return true;
                        break;
                    case Register::Type::String:
                        if (condition->getString() == equal->getString()) return true;
                        break;
                }
        } else {
            if ((condition->getType() == Register::Type::Bool) && (condition->getBool()))
                return true;
        }
    }
}

void Selection::close() {
    input->close();
}

vector<const Register *> Selection::getOutput() const {
    return input->getOutput();
}