#include "operator/Projection.hpp"

using namespace std;

Projection::Projection(
   unique_ptr<Operator>&& input,
   const vector<const Register*>& output)
   : input(move(input)),output(output) {
}

Projection::~Projection() {
}

void Projection::open() {
   input->open();
}

bool Projection::next() {
   return input->next();
}

void Projection::close() {
   input->close();
}

vector<const Register*> Projection::getOutput() const {
   return output;
}
