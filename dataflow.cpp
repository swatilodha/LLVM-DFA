#include "dataflow.h"

using namespace std;

namespace llvm {

void Dataflow::gen_traversal(Function &F) {
  stack<BasicBlock *> rpo_stack;
  po_iterator<BasicBlock *> start = po_begin(&F.getEntryBlock());
  po_iterator<BasicBlock *> end = po_end(&F.getEntryBlock());
  while (start != end) {
    po_traversal.push_back(*start);
    rpo_stack.push(*start);
    ++start;
  }

  while (!rpo_stack.empty()) {
    rpo_traversal.push_back(rpo_stack.top());
    rpo_stack.pop();
  }
}

void Dataflow::init_prev_and_next(BasicBlock *BB, struct bb_props *props) {
  for (BasicBlock *pred_blocks : predecessors(BB)) {
    props->p_blocks.push_back(pred_blocks);
  }
  for (BasicBlock *succ_blocks : successors(BB)) {
    props->s_blocks.push_back(succ_blocks);
  }
}

void Dataflow::init_props(Function &F) {
  BitVector empty(domain_size, false);

  for (BasicBlock &BB : F) {
    struct bb_props *props = new bb_props();
    props->block_ref = &BB;
    props->bb_input = empty;
    props->bb_output = empty;

    // Set GenSet and KillSet
    struct bb_req *req = bb_map[&BB];
    props->gen_set = req->gen_set;
    props->kill_set = req->kill_set;

    // Initialize predecessors and successors for each BasicBlock
    init_prev_and_next(&BB, props);

    // Determine block type - <ENTRY, EXIT, REGULAR>
    if (&BB == &F.getEntryBlock()) {
      props->b_t = ENTRY;
    } else {
      props->b_t = REGULAR;
    }
    for (Instruction &II : BB) {
      Instruction *I = &II;
      if (isa<ReturnInst>(I)) {
        props->b_t = EXIT;
      }
    }
    dfa[&BB] = props;
  }

  map<BasicBlock *, struct bb_props *>::iterator it = dfa.begin();
  struct bb_props *temp;

  // Set initial and boundary conditions for each BasicBlock
  while (it != dfa.end()) {
    temp = dfa[it->first];
    if (dir == FORWARD) {
      if (temp->b_t == ENTRY) {
        temp->bb_input = boundary_cond;
      }
      temp->bb_output = init_cond;
    } else if (dir == BACKWARD) {
      if (temp->b_t == EXIT) {
        temp->bb_output = boundary_cond;
      }
      temp->bb_input = init_cond;
    }
    ++it;
  }
}

void Dataflow::run(Function &F) {
  bool converged = false;
  map<BasicBlock *, BitVector> prev_output;
  init_props(F);
  gen_traversal(F);
  int iter = 0;
  vector<BasicBlock *> traversal =
      (dir == FORWARD) ? po_traversal : rpo_traversal;
  while (!converged) {
    for (BasicBlock *BB : traversal) {
      prev_output[BB] = dfa[BB]->bb_output;
      vector<BitVector> meet_inputs;
      // Apply meet function based on
      if (dir == FORWARD) {
        for (BasicBlock *p : dfa[BB]->p_blocks) {
          meet_inputs.push_back(dfa[p]->bb_output);
        }
      } else if (dir == BACKWARD) {
        for (BasicBlock *s : dfa[BB]->s_blocks) {
          meet_inputs.push_back(dfa[s]->bb_input);
        }
      }
      if (!meet_inputs.empty()) {
        BitVector meet = meet_fn(meet_inputs);
        if (dir == FORWARD) {
          dfa[BB]->bb_input = meet;
        } else if (dir == BACKWARD) {
          dfa[BB]->bb_output = meet;
        }
      }
      transfer_fn(dfa[BB]);
    }
    converged = true;
    for (map<BasicBlock *, BitVector>::iterator it = prev_output.begin();
         it != prev_output.end(); ++it) {
      if ((dfa[it->first]->bb_output) != it->second) {
        converged = false;
        break;
      }
    }
    ++iter;
  }
  outs() << "DFA converged after " << iter << "\n";
}
} // namespace llvm