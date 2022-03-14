// ECE/CS 5544 S22 Assignment 2: reaching.cpp
// Group:

////////////////////////////////////////////////////////////////////////////////

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/raw_ostream.h"

#include "dataflow.h"
#include "available-support.h"

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
        /**
        vector<Expression> expressions;
        for (Function::iterator FI = F.begin(), FE = F.end(); FI != FE; ++FI) {
            BasicBlock* block = &*FI;
            for (BasicBlock::iterator i = block->begin(), e = block->end(); i!=e; ++i) {
                Instruction* I = &*i;
                // We only care about available expressions for BinaryOperators
                if (BinaryOperator *BI = dyn_cast<BinaryOperator>(I)) {
                    if(std::find(expressions.begin(),expressions.end(),Expression(BI)) != expressions.end())
                        continue;
                    else
                        expressions.push_back(Expression(BI));
                 }
            }
        }
        
      
        // Print out the expressions used in the function
        outs() << "Expressions used by this function:\n";
        printSet(&expressions);

        BitVector boundaryCond(expressions.size(), false);
        BitVector initCond(expressions.size(), false);
        dataFlow* df = new dataFlow(expressions.size(),INTERSECTION,FORWARD,boundaryCond,initCond);
        generateDeps(F,expressions);
        df->executeDataFlowPass(F,bbSets);
        printDFAResults(df->dataFlowHash);
        **/
        // Did not modify the incoming Function.
        vector<Value*> domain;
        for (auto arg = F.arg_begin(); arg != F.arg_end(); ++arg) {
            domain.push_back(&*arg);
        }
        for (auto I = inst_begin(F); I != inst_end(F); ++I) {
            if (Value* val = dyn_cast<Value> (&*I)) {
                if (getShortValueName(val) != "") {
                    if (std::find(domain.begin(), domain.end(), val) == domain.end()) {
                        domain.push_back(val);
                    }
                }
            }
        }
        
        BitVector boundaryCond(expressions.size(), false);
        BitVector initCond(expressions.size(), false);
        

        return false;
    }

    virtual void getAnalysisUsage(AnalysisUsage& AU) const {
      AU.setPreservesAll();
    }
    
    private:
    std::map<BasicBlock*,basicBlockDeps*> bbSets;
    std::map<Value*,int> domainMap;
    std::map<int,string> revDomainMap;
    void generateDeps(Function &F, std::vector<Value*> domain)
    {
      BitVector empty(domain.size(), false);
      int vectorIdx = 0;
      StringRef lhs;
      for(Value* val : domain)
      {
        domainMap[val] = vectorIdx;
        revDomainMap[vectorIdx] = (string) val->getName();
        ++vectorIdx;
      }
      for(BasicBlock &BB : F)
      {
        struct basicBlockDeps* bbSet = new basicBlockDeps();
        bbSet->blockRef = &BB;
        bbSet->genSet = empty;
        bbSet->killSet = empty;
        for(Instruction &II : BB)
        {
          Instruction *I = &II;
          if(dyn_cast<BinaryOperator>(I))
          {
            lhs = I->getName();
            if(std::find(domain.begin(),domain.end(),Expression(I)) != domain.end())
            {
              bbSet->genSet.set(domainMap[dyn_cast<Value*>(I)]); 
            }
            for(Expression itr: domain)
            {
              StringRef exprOp1 = itr.v1->getName();
              StringRef exprOp2 = itr.v2->getName();
              if(exprOp1.equals(lhs) || exprOp2.equals(lhs))
              {
                bbSet->killSet.set(domainMap[itr]);
                bbSet->genSet.reset(domainMap[itr]);
              }
            }
          }
        }
        bbSets[&BB] = bbSet;
      }
    }

    void printDFAResults(std::map<BasicBlock*,basicBlockProps*> dFAHash)
    {
      std::map<BasicBlock*,basicBlockProps*>::iterator it = dFAHash.begin();
      while(it != dFAHash.end())
      {
        struct basicBlockProps* temp = dFAHash[it->first];
        outs() << "Basic Block Name : ";
        outs() << temp->block->getName() << "\n";
        std::vector<string> genbb;
        outs() << "gen[BB] : ";
        for(int m = 0; m < (int)temp->genSet.size(); m++)
        {
          if(temp->genSet[m])
          {
            genbb.push_back(revDomainMap[m]);
          }
        }
        printStringSet(&genbb);
        std::vector<string> killbb;
        outs() << "kill[BB] : ";
        for(int n = 0; n < (int)temp->killSet.size(); n++)
        {
          if(temp->killSet[n])
          {
            killbb.push_back(revDomainMap[n]);
          }
        }
        printStringSet(&killbb);
        std::vector<string> inbb;
        outs() << "IN[BB] : ";
        for(int k = 0; k < (int)temp->bbInput.size(); k++)
        {
          if(temp->bbInput[k])
          {
            inbb.push_back(revDomainMap[k]);
          }
        }
        printStringSet(&inbb);
        std::vector<string> outbb;
        outs() << "OUT[BB] : ";
        for(int l = 0; l < (int)temp->bbOutput.size(); l++)
        {
          if(temp->bbOutput[l])
          {
            outbb.push_back(revDomainMap[l]);
          }
        }
        printStringSet(&outbb);
        outs() << "\n";
        it++;
      }
    }
  };
  
  char ReachingDefinitions::ID = 0;
  RegisterPass<ReachingDefinitions> X("reaching", "ECE/CS 5544 Reaching");
}