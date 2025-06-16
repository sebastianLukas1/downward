#ifndef COMPRESSED_PDBS_PDB_HEURISTIC_H
#define COMPRESSED_PDBS_PDB_HEURISTIC_H

#include "compressed_pdb.h"
#include "pattern_generator.h"

#include "../heuristic.h"

class State;

namespace pdbs {
class CompressedPatternDatabase;

// Implements a heuristic for a single PDB.
class CompressedPDBHeuristic : public Heuristic {
    std::shared_ptr<CompressedPatternDatabase> pdb;
    const State *predecessor_state;
    const State *successor_state;
protected:
    virtual int compute_heuristic(const State &ancestor_state) override;
public:
    /*
      Important: It is assumed that the pattern (passed via
      pattern_generator) is sorted, contains no duplicates and is small
      enough so that the number of abstract states is below
      numeric_limits<int>::max()
      Parameters:
       operator_costs: Can specify individual operator costs for each
       operator. This is useful for action cost partitioning. If left
       empty, default operator costs are used.
    */
    CompressedPDBHeuristic(
        const std::shared_ptr<PatternGenerator> &pattern_generator,
        const std::shared_ptr<AbstractTask> &transform,
        bool cache_estimates, const std::string &description,
        utils::Verbosity verbosity);

    virtual void get_path_dependent_evaluators(
        std::set<Evaluator*>& evals) override {
        evals.insert(this);
    }

    virtual void notify_state_transition(
        const State &parent_state,
        OperatorID op_id,
        const State &state) override;
};
}

#endif
