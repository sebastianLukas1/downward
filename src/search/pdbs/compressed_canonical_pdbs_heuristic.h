#ifndef PDBS_COMPRESSED_CANONICAL_PDBS_HEURISTIC_H
#define PDBS_COMPRESSED_CANONICAL_PDBS_HEURISTIC_H

#include "compressed_canonical_pdbs.h"
#include "pattern_generator.h"

#include "../heuristic.h"

namespace plugins {
class Feature;
}

namespace pdbs {
// Implements the canonical heuristic function.
class CompressedCanonicalPDBsHeuristic : public Heuristic {
    CompressedCanonicalPDBs canonical_pdbs;

protected:
    virtual int compute_heuristic(const State &ancestor_state) override;

public:
    CompressedCanonicalPDBsHeuristic(
        const std::shared_ptr<PatternCollectionGenerator> &patterns,
        double max_time_dominance_pruning,
        const std::shared_ptr<AbstractTask> &transform,
        bool cache_estimates, const std::string &description,
        utils::Verbosity verbosity);
};

void add_canonical_pdbs_options_to_feature(plugins::Feature &feature);
std::tuple<double> get_canonical_pdbs_arguments_from_options(
    const plugins::Options &opts);
}

#endif
