#include "operator/Tablescan.hpp"
#include "Table.hpp"
#include <cstdlib>

using namespace std;

Tablescan::Tablescan(
   Table& table)
   : table(table),
      bufferStart(0),
      bufferStop(0),
      filePos(0) {
   output.resize(table.getAttributeCount());
}

Tablescan::~Tablescan() {}

void Tablescan::open() {
   bufferStart=bufferStop=0; filePos=0;
}

bool Tablescan::next() {
   bool escape=false;
   for (unsigned index=0,limit=output.size();index<limit;++index) {
      buf.resize(0);
      while (true) {
         if (bufferStart>=bufferStop) {
            // get end position
            table.io.seekg(0,ios_base::end);
            std::streamsize tsize = table.io.tellg();

            // set to filePos again
            table.io.seekg(filePos,ios_base::beg);

            // calculate size of available data
            std::streamsize lsize = tsize - filePos; // could catch errors here

            // limit lookup to buffer size
            std::streamsize lookup = std::min<std::streamsize>(lsize, bufferSize);

            // if read fails abort
            if (lookup == 0 || !(table.io.read(buffer, lookup))){
               //cout << "read failed" << endl;
               table.io.clear();
               return false;
            }

            // set new lookup
            bufferStart = 0;
            bufferStop = lookup;
            filePos += lookup;

            /*
            cout << "filepos: " << filePos << endl;
            table.io.seekg(filePos,ios_base::beg); // begin to read from filePos
            cout << "buffersize: " << bufferSize << endl;
            char test = table.io.peek();
            cout << "peeked: " << test << endl;
            

            int len=table.io.readsome(buffer,bufferSize); // read all in buffer
            if (len<1) { cout << "buffer len was: " << len << endl; table.io.clear(); return false;  }

            bufferStart=0;
            bufferStop=len;
            filePos+=len;*/
         }
         char c=buffer[bufferStart++];
         if (escape) {
            //cout << "got escape" << endl;
            escape=false; buf+=c; continue; }
         if (c=='\r')
         {
            //cout << "got return" << endl;
            continue;
         }
         if ((c==';')||(c=='\n')) {

            // cout << "got register " << index << " with value: " << buf << endl;
            Register& r=output[index];
            switch (table.attributes[index].getType()) {
               case Attribute::Type::Int: r.setInt(atoi(buf.c_str())); break;
               case Attribute::Type::Double: r.setDouble(atof(buf.c_str())); break;
               case Attribute::Type::Bool: r.setBool(buf=="true"); break;
               case Attribute::Type::String: r.setString(buf); break;
            }
            break;
         } else if (c=='\\') {
            escape=true;
         } else buf+=c;
      }
   }

   return true;
}

void Tablescan::close() {}

vector<const Register*> Tablescan::getOutput() const {
   vector<const Register*> result;
   for (auto iter=output.begin(),limit=output.end();iter!=limit;++iter)
      result.push_back(&(*iter));
   return result;
}

const Register* Tablescan::getOutput(const std::string& name) const {
   int slot=table.findAttribute(name);
   if (slot<0) return 0;
   return &output[slot];
}
