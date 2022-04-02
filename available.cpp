// ECE/CS 5544 S22 Assignment 2: available.cpp
// Group: Swati Lodha, Abhijit Tripathy

////////////////////////////////////////////////////////////////////////////////

#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include "available-support.h"
#include "dataflow.h"

using namespace llvm;
using namespace std;

namespace llvm {

void transfer_fn(struct bb_props *props) {
  BitVector kill_set = props->kill_set;
  props->bb_output = kill_set.flip();
  props->bb_output &= props->bb_input;
  props->bb_output |= props->gen_set;
}

BitVector meet_fn(vector<BitVector> input) {
  size_t _sz = input.size();

  BitVector result = input[0];

  // Meet Function : Intersection
  // result stores bitwise AND of all input BitVectors
  for(int i=1;i<_sz;i++) {
    result &= input[i];
  }

  return result;
}

class AvailableExpressions : public FunctionPass {
public:
  static char ID;
  AvailableExpressions() : FunctionPass(ID) {}

  virtual bool runOnFunction(Function &F) {
    vector<Expression> expressions;
    for (Function::iterator FI = F.begin(), FE = F.end(); FI != FE; ++FI) {
      BasicBlock *block = &*FI;
      for (BasicBlock::iterator i = block->begin(), e = block->end(); i != e;
           ++i) {
        Instruction *I = &*i;
        // We only care about available expressions for BinaryOperators
        if (BinaryOperator *BI = dyn_cast<BinaryOperator>(I)) {
          if (find(expressions.begin(), expressions.end(),
                        Expression(BI)) != expressions.end())
            continue;
          else
            expressions.push_back(Expression(BI));
        }
      }
    }

    outs() << "Expressions used by this function:\n";
    printSet(&expressions);

    BitVector boundary_cond(expressions.size(), false);
    BitVector init_cond(expressions.size(), true);

    init_map(F, expressions);

    Dataflow *df = new Dataflow(
      expressions.size(), 
      BACKWARD, 
      meet_fn, 
      transfer_fn, 
      bb_map, 
      init_cond, 
      boundary_cond
    );

    df->run(F);
    printResults(df->dfa);
    return false;
  }

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
  }

private:
  map<BasicBlock *, struct bb_req *> bb_map;
  map<Expression, int> domain_map;
  map<int, string> rev_domain_map;


  void init_map(Function &F, vector<Expression> domain) {
    BitVector empty(domain.size(), false);
    int idx = 0;
    StringRef lhs;

    for(Expression e : domain) {
      domain_map[e] = idx;
      rev_domain_map[idx] = e.toString();
      ++idx;
    }

    for(BasicBlock &BB: F) {
      struct bb_req *info = new bb_req();
      info->block_ref = &BB;
      info->gen_set = empty;
      info->kill_set = empty;

      for(Instruction &II : BB) {
        Instruction *I = &II;
        if(dyn_cast<BinaryOperator>(I)) {
          Expression exp = Expression(I);
          lhs = I->getName();
          if(find(domain.begin(), domain.end(), exp) != domain.end()) {
            info->gen_set.set(domain_map[exp]);
          }

          for(Expression e : domain) {
            StringRef e_op1 = e.v1->getName();
            StringRef e_op2 = e.v2->getName();

            if(e_op1.equals(lhs) || e_op2.equals(lhs)) {
              info->kill_set.set(domain_map[e]);
              info->gen_set.reset(domain_map[e]);
            }
          }
        }
      }
      bb_map[&BB] = info;
    }
  }

  void printResults(map<BasicBlock *, struct bb_props *> dfa) {
    map<BasicBlock *, struct bb_props *>::iterator it = dfa.begin();
    while (it != dfa.end()) {
      struct bb_props *temp = dfa[it->first];
      outs() << "Basic Block Name : ";
      outs() << temp->block_ref->getName() << "\n";
      vector<string> genbb;
      outs() << "gen[BB] : ";
      for (int m = 0; m < (int)temp->gen_set.size(); m++) {
        if (temp->gen_set[m]) {
          genbb.push_back(rev_domain_map[m]);
        }
      }
      printStringSet(&genbb);
      vector<string> killbb;
      outs() << "kill[BB] : ";
      for (int n = 0; n < (int)temp->kill_set.size(); n++) {
        if (temp->kill_set[n]) {
          killbb.push_back(rev_domain_map[n]);
        }
      }
      printStringSet(&killbb);
      vector<string> inbb;
      outs() << "IN[BB] : ";
      for (int k = 0; k < (int)temp->bb_input.size(); k++) {
        if (temp->bb_input[k]) {
          inbb.push_back(rev_domain_map[k]);
        }
      }
      printStringSet(&inbb);
      vector<string> outbb;
      outs() << "OUT[BB] : ";
      for (int l = 0; l < (int)temp->bb_output.size(); l++) {
        if (temp->bb_output[l]) {
          outbb.push_back(rev_domain_map[l]);
        }
      }
      printStringSet(&outbb);
      outs() << "\n";
      it++;
    }
  }
};

char AvailableExpressions::ID = 0;
RegisterPass<AvailableExpressions> X("available",
                                     "ECE/CS 5544 Available Expressions");
}