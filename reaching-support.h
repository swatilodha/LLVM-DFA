
////////////////////////////////////////////////////////////////////////////////

#ifndef __AVAILABLE_SUPPORT_H__
#define __AVAILABLE_SUPPORT_H__

#include <string>
#include <vector>

#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

namespace llvm {
  using namespace std;
  std::string getShortValueName1(Value * v);

  class Definition {
  public:
    Value* v1;
    Value* v2;
    Value* lhs;
    Instruction::BinaryOps op;
    Definition (Instruction * I);
    bool operator== (const Definition &d2) const;
    bool operator< (const Definition &d2) const;
    std::string toString() const;
  };

  void printSet1(std::vector<Definition> * x);
  void printStringSet1(std::vector<string> * x);
}

#endif