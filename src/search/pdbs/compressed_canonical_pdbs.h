#ifndef PDBS_COMPRESSED_CANONICAL_PDBS_H
#define PDBS_COMPRESSED_CANONICAL_PDBS_H

#include "types.h"

#include <memory>

class State;

namespace pdbs {
class CompressedCanonicalPDBs {
    std::shared_ptr<CompressedPDBCollection> pdbs;
    std::shared_ptr<std::vector<PatternClique>> pattern_cliques;

public:
    CompressedCanonicalPDBs(
        const std::shared_ptr<PDBCollection>& pdbs,
        const std::shared_ptr<std::vector<PatternClique>>& pattern_cliques,
        const std::vector<int> initial_state_values);
    ~CompressedCanonicalPDBs() = default;

    int get_value(const State &state, const State &predecessor_state) const;

    int get_cache_size();
};
}

#endif
