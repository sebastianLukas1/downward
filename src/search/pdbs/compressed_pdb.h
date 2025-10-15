#ifndef PDBS_COMPRESSED_PATTERN_DATABASE_H
#define PDBS_COMPRESSED_PATTERN_DATABASE_H

#include "pattern_database.h"
#include "types.h"

#include "../task_proxy.h"

#include <vector>

namespace pdbs {


class CompressedPatternDatabase {
    Projection projection;
    
    int initial_state_heuristic_value;
public:
    /*
      compressed h values for 5 states each
    */
    std::vector<unsigned char> distances;
    std::unordered_map<int, int> cached_values;

    CompressedPatternDatabase(
        const PatternDatabase &pdb, std::vector<int> initial_state_values);
    int get_value(const std::vector<int>& state) const;
    int get_full_value(const std::vector<int> &state, const std::vector<int> &predecessor_state);

    int get_initial_state_heuristic_value() {
        return this->initial_state_heuristic_value;
    };

    const Pattern &get_pattern() const {
        return projection.get_pattern();
    }

    // The size of the PDB is the number of abstract states.
    int get_size() const {
        return projection.get_num_abstract_states();
    }

    /*
      Return the average h-value over all states, where dead-ends are
      ignored (they neither increase the sum of all h-values nor the
      number of entries for the mean value calculation). If all states
      are dead-ends, return infinity.
      Note: This is only calculated when called; avoid repeated calls to
      this method!
    */
    double compute_mean_finite_h() const;

    int get_cache_size();
};
}

#endif
