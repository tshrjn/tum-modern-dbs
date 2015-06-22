#include "operator/HashJoin.hpp"

using namespace std;


HashJoin::HashJoin(
   std::unique_ptr<Operator>&& left,
   std::unique_ptr<Operator>&& right,
   const Register* leftValue,
   const Register* rightValue)
   : left(move(left)),
      right(move(right)),
      leftValue(leftValue),
      rightValue(rightValue)
{
   vector<const Register*> lr(this->left->getOutput());
   for (auto iter=lr.begin(),limit=lr.end();iter!=limit;++iter)
      leftRegs.push_back(const_cast<Register*>(*iter));
}

HashJoin::~HashJoin() {}

// The open call of the hashjoin will clean the hashtable as
// everything needs to be (re)hashed
void HashJoin::open() {
   left->open();
   right->open();
   table.clear();
}


// In the first run we need to build the complete hash table
// Subsequently we just 
bool HashJoin::next() {
   // At the first run we need to build the complete hash table with the left side
   if (table.empty()) {
      while (left->next()) {
         vector<Register> values;
         values.reserve(leftRegs.size());
         // Copy the Registers and store them in a values vector
         for (auto iter=leftRegs.begin(),limit=leftRegs.end();iter!=limit;++iter)
            values.push_back(**iter);
         // Then store everything into a multimap
         table.insert(make_pair(*leftValue,move(values)));
      }
      if (table.empty()) return false;
      iter=iterLimit=table.end();
   }

   while (true) {
      // AFTER equal_range found a range of equal keys the tuples are produced iteratively
      // Until the lower iterator hits the upper border again
      // Meanwhile the right values stay untouched
      if (iter!=iterLimit) {
         const vector<Register>& values=(*iter).second;
         auto iterValues=values.begin();
         // Copy the found values to the result registers
         for (auto iterRegs=leftRegs.begin(),limitRegs=leftRegs.end();
               iterRegs!=limitRegs;
               ++iterRegs,++iterValues)
            **iterRegs=*iterValues;
         ++iter;
         return true;
      }
      if (!right->next()) {
         right->close();
         return false;
      }

      // Range of all elements in the table that have a key equal to the right register
      auto leftRange=table.equal_range(*rightValue);

      // The next tuple will be the range.first and right
      iter=leftRange.first; iterLimit=leftRange.second;
   }
}

// Clear the hashtable again on close
void HashJoin::close() {
   if (!table.empty()) {
      right->close();
      table.clear();
   }
   left->close();
}

// Merge the left and right output togeher into one vector
// TODO: Join attribute is in here twice
vector<const Register*> HashJoin::getOutput() const {
   vector<const Register*> result=left->getOutput(),other=right->getOutput();
   for (auto iter=other.begin(),limit=other.end();iter!=limit;++iter)
      result.push_back(*iter);
   return result;
}