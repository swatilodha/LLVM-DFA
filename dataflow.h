#ifndef __CLASSICAL_DATAFLOW_H__
#define __CLASSICAL_DATAFLOW_H__

#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <queue>
#include <stack>
#include <stdio.h>
#include <vector>

using namespace std;

namespace llvm {

enum pass_direction { FORWARD = 0, BACKWARD = 1 };

enum meet { UNION = 0, INTERSECTION = 1 };

enum block_type {
  ENTRY = 0,
  EXIT = 1,
  REGULAR = 2,
};

struct bb_req { // struct for each BasicBlock's gen and kill sets that are
                // required as inputs for dataflow analysis
  BasicBlock *block_ref;
  BitVector gen_set;
  BitVector kill_set;
};

struct bb_props {
  enum block_type b_t;
  BasicBlock *block_ref;
  BitVector bb_input;
  BitVector bb_output;
  BitVector gen_set;             // Gen Set for each block
  BitVector kill_set;            // Kill set for each block
  vector<BasicBlock *> p_blocks; // Predecessor Blocks
  vector<BasicBlock *> s_blocks; // Successor Blocks
};

class Dataflow {
  private:
    int domain_size;
    enum pass_direction dir;
    map<BasicBlock *, struct bb_req *> bb_map;
    BitVector (*meet_fn)(vector<BitVector>);
    void (*transfer_fn)(struct bb_props *);
    BitVector init_cond;
    BitVector boundary_cond;
    vector<BasicBlock *> po_traversal;
    vector<BasicBlock *> rpo_traversal;
    void init_props(Function &F);
    void init_prev_and_next(BasicBlock *BB, struct bb_props *props);
    void gen_traversal(Function &F);

  public:
    map<BasicBlock *, bb_props *> dfa; // Dataflow Analysis result map
    
    Dataflow(int domain_size, 
            enum pass_direction dir,
            BitVector (*meet_fn)(vector<BitVector>),
            void (*transfer_fn)(struct bb_props *),
            map<BasicBlock *, struct bb_req *> bb_map, 
            BitVector init_cond,
            BitVector boundary_cond) { // Initialize the DFA

      this->domain_size = domain_size;
      this->dir = dir;
      this->meet_fn = meet_fn;
      this->transfer_fn = transfer_fn;
      this->bb_map = bb_map;
      this->init_cond = init_cond;
      this->boundary_cond = boundary_cond;
    }
    void run(Function &F);
  };
} // namespace llvm

#endif
