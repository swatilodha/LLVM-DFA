// ECE/CS 5544 S22 Assignment 2: available.cpp
// Group: Swati Lodha, Abhijit Tripathy

////////////////////////////////////////////////////////////////////////////////

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/raw_ostream.h"

#include "dataflow.h"
#include "reaching-support.h"

using namespace llvm;
using namespace std;

namespace {
  class ReachingDefinitions : public FunctionPass {
    public:
    static char ID;
    ReachingDefinitions() : FunctionPass(ID) { }
    
    virtual bool runOnFunction(Function& F) {

        // Here's some code to familarize you with the Expression
        // class and pretty printing code we've provided:

        vector<Definition> definitions;
        for (Function::iterator FI = F.begin(), FE = F.end(); FI != FE; ++FI) {
            BasicBlock* block = &*FI;
            for (BasicBlock::iterator i = block->begin(), e = block->end(); i!=e; ++i) {
                Instruction* I = &*i;
                // We only care about definitions for BinaryOperators
                if (BinaryOperator *BI = dyn_cast<BinaryOperator>(I)) {
                    if(std::find(definitions.begin(), definitions.end(), Definition(BI)) != definitions.end())
                        continue;
                    else
                        definitions.push_back(Definition(BI));
                 }
            }
        }
        
      
        // Print out the definitions used in the function
        outs() << "Definitions used by this function:\n";
        printSet1(&definitions);

        BitVector boundaryCond(definitions.size(), false);
        BitVector initCond(definitions.size(), true);
        dataFlow* df = new dataFlow(definitions.size(), UNION, FORWARD, boundaryCond, initCond);
        initializeDeps(F,definitions);
        df->executeDataFlowPass(F,bbSets);
        printResults(df->dataFlowHash);

        // Did not modify the incoming Function.
        return false;
    }

    virtual void getAnalysisUsage(AnalysisUsage& AU) const {
      AU.setPreservesAll();
    }
    
    private:
    std::map<BasicBlock*,basicBlockDeps*> bbSets;
    std::map<Definition,int> domainMap;
    std::map<int,string> revDomainMap;
    void initializeDeps(Function &F, std::vector<Definition> domain) {
      BitVector empty((int)domain.size(), false);
      int vectorIdx = 0;
      StringRef lhs;
      for(Definition d : domain) {
        domainMap[d] = vectorIdx;
        revDomainMap[vectorIdx] = d.toString();
        ++vectorIdx;
      }
      for(BasicBlock &BB : F) {
        struct basicBlockDeps* bbSet = new basicBlockDeps();
        bbSet->blockRef = &BB;
        bbSet->genSet = empty;
        bbSet->killSet = empty;
        for(BasicBlock::iterator II=BB.begin(); II != BB.end(); ++II) {
          Instruction *I = &(*II);
          if(BinaryOperator* BO1 = dyn_cast<BinaryOperator>(I)) {
            outs() << "Value name : " << I->getName() << "\n";
            string lhs = getShortValueName1(I);
            if(std::find(domain.begin(), domain.end(), Definition(BO1)) != domain.end()) {
              bbSet->genSet.set(domainMap[Definition(BO1)]); 
            }
            // Calculate kill[BB] : Set of definitions in the domain that are killed by the current Basic Block
            for(Definition itr: domain) {
              string s_lhs = getShortValueName1(itr.lhs);
              if(lhs == s_lhs) {
                bbSet->killSet.set(domainMap[itr]);
                // bbSet->genSet.reset(domainMap[itr]);
              }
            }
            for(BasicBlock::iterator prev = BB.begin(); prev != II; ++prev) {
                Instruction* prevInst = &(*prev);
                if(BinaryOperator* BO_prev = dyn_cast<BinaryOperator>(prevInst)){
                    string prev_lhs = getShortValueName1(prevInst);
                    if(prev_lhs == lhs) {
                        bbSet->genSet.reset(domainMap[Definition(prevInst)]);
                    }
                }
            }
          }
        }
        bbSets[&BB] = bbSet;
      }
    }

    void printResults(std::map<BasicBlock*,basicBlockProps*> dFAHash)
    {
      std::map<BasicBlock*,basicBlockProps*>::iterator it = dFAHash.begin();
      while(it != dFAHash.end()) {
        struct basicBlockProps* temp = dFAHash[it->first];
        outs() << "Basic Block Name : ";
        outs() << temp->block->getName() << "\n";
        std::vector<string> genbb;
        outs() << "gen[BB] : ";
        for(int m = 0; m < (int)temp->genSet.size(); m++) {
          if(temp->genSet[m])
          {
            genbb.push_back(revDomainMap[m]);
          }
        }
        printStringSet1(&genbb);
        std::vector<string> killbb;
        outs() << "kill[BB] : ";
        for(int n = 0; n < (int)temp->killSet.size(); n++) {
          if(temp->killSet[n])
          {
            killbb.push_back(revDomainMap[n]);
          }
        }
        printStringSet1(&killbb);
        std::vector<string> inbb;
        outs() << "IN[BB] : ";
        for(int k = 0; k < (int)temp->bbInput.size(); k++){
          if(temp->bbInput[k])
          {
            inbb.push_back(revDomainMap[k]);
          }
        }
        printStringSet1(&inbb);
        std::vector<string> outbb;
        outs() << "OUT[BB] : ";
        for(int l = 0; l < (int)temp->bbOutput.size(); l++) {
          if(temp->bbOutput[l])
          {
            outbb.push_back(revDomainMap[l]);
          }
        }
        printStringSet1(&outbb);
        outs() << "\n";
        it++;
      }
    }
    };

    char ReachingDefinitions::ID = 0;
    RegisterPass<ReachingDefinitions> X("reaching",
		           "ECE/CS 5544 Reaching Definitions");
}