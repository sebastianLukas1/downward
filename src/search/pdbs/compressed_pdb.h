#ifndef PDBS_COMPRESSED_PATTERN_DATABASE_H
#define PDBS_COMPRESSED_PATTERN_DATABASE_H

#include "pattern_database.h"
#include "types.h"

#include "../task_proxy.h"

#include <vector>

namespace pdbs {


class CompressedPatternDatabase {
    Projection projection;

    /*
      compressed h values for 5 states each
    */
    std::vector<char> distances;
public:
    CompressedPatternDatabase(
        const PatternDatabase &pdb);
    int get_value(const std::vector<int> &state) const;

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
};
}

#endif
