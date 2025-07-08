#include "compressed_pdb.h"

#include "../task_utils/task_properties.h"

#include "../utils/logging.h"
#include "../utils/math.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

using namespace std;

namespace pdbs {
    static int decompress_heuristic_value(int compressed_h, int predecessor_h) {
        int compressed_predecessor_h = predecessor_h % 3;

        //both h values are the same
        if (compressed_h == compressed_predecessor_h) {
            return predecessor_h;
        }
        //check if h is lower than predecessor_h
        if ((compressed_h == 0 && compressed_predecessor_h == 1) ||
            (compressed_h == 1 && compressed_predecessor_h == 2) ||
            (compressed_h == 2 && compressed_predecessor_h == 0)) {
            return predecessor_h - 1;
        }
        //otherwise h is higher than predecessor_h
        return predecessor_h + 1;
    }

CompressedPatternDatabase::CompressedPatternDatabase(
    const PatternDatabase &pdb, std::vector<int> initial_state_values)
    : projection(pdb.getProjection()),
      distances(static_cast<int>(ceil(pdb.get_size() / 5.0))),
      cached_values() {
    cout << "   TEST: CPDB construction started" << endl;
    this->initial_state_heuristic_value = pdb.get_value(initial_state_values);
    int initial_state_index = projection.rank(initial_state_values);
    cout << "   TEST: index of initial state: " << initial_state_index << endl;
    this->cached_values[initial_state_index] = this->initial_state_heuristic_value;
    cout << "   TEST: added initial_state_heuristic_value to cache" << endl;

    for (int i = 0; i < pdb.get_size(); i++) {
        int index = i / 5;
        int subindex = i % 5;
        int compressed_h_value = pdb.distances[i] % 3;
        this->distances[index] += compressed_h_value * pow(3, subindex);
    }
    cout << "   TEST: CPDB construction complete" << endl;
}

int CompressedPatternDatabase::get_value(const vector<int> &state) const {
    int index = projection.rank(state);
    int i = index / 5;
    int subindex = index % 5;
    unsigned char values = distances[i];
    int result = static_cast<int>(values / pow(3, subindex)) % 3;
    return result;
}

int CompressedPatternDatabase::get_full_value(const std::vector<int>& state, const std::vector<int>& predecessor_state) {
    int predecessor_index = projection.rank(predecessor_state);
    int predecessor_h = cached_values[predecessor_index];
    if (predecessor_h == 0) {
        cout << "       ERROR: the predecessor has h=0";
        exit(EXIT_FAILURE);
    }

    int index = projection.rank(state);
    int compressed_h = get_value(state);
    int h = decompress_heuristic_value(compressed_h, predecessor_h);

    this->cached_values[index] = h;
    return h;
}

double CompressedPatternDatabase::compute_mean_finite_h() const {
    double sum = 0;
    int size = 0;
    for (size_t i = 0; i < distances.size(); ++i) {
        if (distances[i] != numeric_limits<int>::max()) {
            sum += distances[i];
            ++size;
        }
    }
    if (size == 0) { // All states are dead ends.
        return numeric_limits<double>::infinity();
    } else {
        return sum / size;
    }
}
}
