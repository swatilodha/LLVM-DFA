// ECE/CS 5544 S22 Assignment 3: dominators.cpp
// Group: Swati Lodha, Abhijit Tripathy

////////////////////////////////////////////////////////////////////////////////

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include "dataflow.h"

using namespace llvm;
using namespace std;

namespace llvm {

class Dominators : public FunctionPass {
public:
  static char ID;
  Dominators() : FunctionPass(ID) {}

  virtual bool runOnFunction(Function &F) {

    vector<string> domain;
    for (BasicBlock &BB : F) {
      domain.push_back(BB.getName().str());
    }

    populateInfoMap(F, domain);

    BitVector boundaryCond(domain.size(), false);
    BitVector initCond(domain.size(), true);

    Analysis *df = new Analysis(domain.size(), FORWARD, boundaryCond, initCond);
    df->run(F, infoMap);
    // Dataflow *df = new Dataflow(domain.size(), infoMap, )
    printResults(df->result);

    return false;
  }

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
    AU.addRequired<LoopInfoWrapperPass>();
  }

private:
  map<BasicBlock *, struct bbInfo *> infoMap;
  map<string, int> domainToBitMap;
  map<int, string> bitToDomainMap;

  void populateInfoMap(Function &F, vector<string> domain);
  void printResults(map<BasicBlock *, struct bbProps *> dfa);
  void printSet(vector<string> list);

  void printSet(vector<string> list) {
    bool first = true;
    outs() << "{";
    for (string str : list) {
      if (!first) {
        outs() << ", ";
      } else {
        first = false;
      }
      outs() << str;
    }
    outs() << "}\n";
  }

  void printResults(map<BasicBlock *, struct bbProps *> dfa) {
    LoopInfo &loop_info = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    int ct = 0;
    for (Loop *loop : loop_info) {
      outs() << "Loop " << ct << " :\n\n";
      for (BasicBlock *bb : loop->getBlocksVector()) {
        outs() << "Basic Block : " << bb->getName() << " :\n";
        struct bbProps *props = dfa[bb];
        vector<string> output;
        for (int idx = 0; idx < props->bbOutput.size(); ++idx) {
          if (props->bbOutput[idx]) {
            output.push_back(bitToDomainMap[idx]);
          }
        }
        printSet(output);
        outs() << "\n";
      }
    }
  }

  void populateInfoMap(Function &F, vector<string> domain) {
    BitVector empty(domain.size(), false);
    int idx = 0;

    for (string blk : domain) {
      domainToBitMap[blk] = idx;
      bitToDomainMap[idx] = blk;
      ++idx;
    }

    for (BasicBlock &BB : F) {
      struct bbInfo *info = new bbInfo();
      info->ref = &BB;
      info->genSet = empty;
      info->killSet = empty;
      info->genSet.set(domainToBitMap[BB.getName().str()]);
      infoMap[&BB] = info;
    }
  }

  class Analysis : public Dataflow {
  public:
    Analysis(int domainSize, enum passDirection dir, BitVector boundaryCond,
             BitVector initCond)
        : Dataflow(domainSize, dir, boundaryCond, initCond) {}

    virtual void transferFn(struct bbProps *props) {
      BitVector tmp = props->bbInput;
      props->bbInput |= props->genSet;
      props->bbOutput = props->bbInput;
      props->bbInput = tmp;
    }

    virtual BitVector meetFn(vector<BitVector> inputs) {
      size_t _sz = inputs.size();
      BitVector result = inputs[0];
      for (int itr = 1; itr < _sz; ++itr) {
        result &= inputs[itr];
      }
      return result;
    }
  };
};

char Dominators::ID = 0;
RegisterPass<Dominators> X("dominators", "ECE/CS 5544 Available Expressions");
} // namespace llvm