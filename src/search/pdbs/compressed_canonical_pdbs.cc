#include "compressed_canonical_pdbs.h"

#include "compressed_pdb.h"
#include "pattern_database.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>

using namespace std;

namespace pdbs {
CompressedCanonicalPDBs::CompressedCanonicalPDBs(
    const shared_ptr<PDBCollection> &pdbs,
    const shared_ptr<vector<PatternClique>> &pattern_cliques,
    const vector<int> initial_state_values)
    : pdbs(new vector<shared_ptr<CompressedPatternDatabase>>()), pattern_cliques(pattern_cliques) {
    this->pdbs->reserve(pdbs->size());
    for (const shared_ptr<PatternDatabase>& pdb : *pdbs) {
        shared_ptr<CompressedPatternDatabase> cpdb = make_shared<CompressedPatternDatabase>(*pdb, initial_state_values);
        this->pdbs->push_back(cpdb);
    }
    
    assert(pdbs);
    assert(pattern_cliques);
}

int CompressedCanonicalPDBs::get_value(const State &state, const State& predecessor_state) const {
    // If we have an empty collection, then pattern_cliques = { \emptyset }.
    assert(!pattern_cliques->empty());
    int max_h = 0;
    vector<int> h_values;
    h_values.reserve(pdbs->size());
    state.unpack();
    predecessor_state.unpack();
    cout << "           TEST: compute h with state " << state.get_id() << " and predecessor " << predecessor_state.get_id() << endl;
    for (const shared_ptr<CompressedPatternDatabase> &pdb : *pdbs) {
        int h = pdb->get_full_value(state.get_unpacked_values(), predecessor_state.get_unpacked_values());
        if (h == numeric_limits<int>::max()) {
            return numeric_limits<int>::max();
        }
        h_values.push_back(h);
    }
    for (const PatternClique &clique : *pattern_cliques) {
        int clique_h = 0;
        for (PatternID pdb_index : clique) {
            clique_h += h_values[pdb_index];
        }
        max_h = max(max_h, clique_h);
    }
    return max_h;
}
}
