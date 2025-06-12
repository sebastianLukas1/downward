#include "compressed_pdb.h"

#include "../task_utils/task_properties.h"

#include "../utils/logging.h"
#include "../utils/math.h"

#include <cassert>
#include <iostream>
#include <limits>
#include <vector>

using namespace std;

namespace pdbs {

CompressedPatternDatabase::CompressedPatternDatabase(
    const PatternDatabase &pdb)
    : projection(pdb.getProjection()),
      distances(static_cast<int>(ceil(pdb.get_size() / 5.0))) {

    for (size_t i = 0; i < distances.size(); i++) {
        int index = i / 5;
        int subindex = i % 5;
        int compressed_h_value = pdb.distances[i] % 3;
        this->distances[index] += compressed_h_value * pow(3, subindex);
    }

    //cout << "     TEST: Compressed PDB constructed\n";
}

int CompressedPatternDatabase::get_value(const vector<int> &state) const {
    int index = projection.rank(state) / 5;
    int subindex = projection.rank(state) % 5;
    char values = distances[index];
    int result = ((int) (((int)values) / pow(3, subindex))) % ((int) pow(3, subindex + 1));
    return result;
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
