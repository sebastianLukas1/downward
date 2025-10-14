#include "compressed_pdb_heuristic.h"

#include "pattern_database.h"

#include "../plugins/plugin.h"
#include "../utils/markup.h"
#include "../utils/timer.h"

#include <limits>
#include <memory>

class State;

using namespace std;

namespace pdbs {
static shared_ptr<CompressedPatternDatabase> get_pdb_from_generator(
    const shared_ptr<AbstractTask> &task,
    const shared_ptr<PatternGenerator> &pattern_generator) {
    utils::Timer timer;
    PatternInformation pattern_info = pattern_generator->generate(task);
    shared_ptr<PatternDatabase> pdb = pattern_info.get_pdb();
    shared_ptr<CompressedPatternDatabase> cpdb = make_shared<CompressedPatternDatabase>(*pdb, task->get_initial_state_values());

    cout << "PDB build time: " << timer() << endl;
    int pdb_memory = sizeof(CompressedPatternDatabase) + cpdb->distances.size();
    cout << "PDB memory: " << pdb_memory << endl;
    return cpdb;
}

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

CompressedPDBHeuristic::CompressedPDBHeuristic(
    const shared_ptr<PatternGenerator> &pattern,
    const shared_ptr<AbstractTask> &transform, bool cache_estimates,
    const string &description, utils::Verbosity verbosity)
    : Heuristic(transform, cache_estimates, description, verbosity),
      pdb(get_pdb_from_generator(task, pattern)),
      predecessor_state(nullptr),
      successor_state(nullptr) {
    /*if (!does_cache_estimates()) {
        exit(EXIT_FAILURE);
    }*/
}

int CompressedPDBHeuristic::compute_heuristic(const State &ancestor_state) {
    //check if the heuristic value is already cached
    /*if (is_estimate_cached(ancestor_state)) {
        return get_cached_estimate(ancestor_state);
    }*/
    //if there is no predecessor state, we are at the initial state
    if (this->predecessor_state == nullptr) {
        return pdb->get_initial_state_heuristic_value();
    }

    State state = convert_ancestor_state(ancestor_state);
    //TODO: remove or throw exception
    if (ancestor_state.get_id() != successor_state->get_id()) {
        cout << "       ERROR: wrong state, got " << ancestor_state.get_id() << " instead of " << successor_state->get_id() << endl;
    }

    /*int predecessor_h = compute_heuristic(*predecessor_state);
    int compressed_h = pdb->get_value(state.get_unpacked_values());
    int h = decompress_heuristic_value(compressed_h, predecessor_h);*/

    state.unpack();
    std::vector<int> unpacked_state = state.get_unpacked_values();
    this->predecessor_state->unpack();
    std::vector<int> unpacked_predecessor = this->predecessor_state->get_unpacked_values();
    int h = pdb->get_full_value(unpacked_state, unpacked_predecessor);
    if (h == numeric_limits<int>::max())
        return DEAD_END;
    return h;
}
void CompressedPDBHeuristic::notify_state_transition(
    const State &parent_state,
    OperatorID op_id,
    const State &state) {
    this->predecessor_state = &parent_state;
    this->successor_state = &state;
}

static basic_string<char> paper_references() {
    return utils::format_conference_reference(
        {"Stefan Edelkamp"},
        "Planning with Pattern Databases",
        "https://aaai.org/papers/7280-ecp-01-2001/",
        "Proceedings of the Sixth European Conference on Planning (ECP 2001)",
        "84-90",
        "AAAI Press",
        "2001") +
           "For implementation notes, see:" + utils::format_conference_reference(
        {"Silvan Sievers", "Manuela Ortlieb", "Malte Helmert"},
        "Efficient Implementation of Pattern Database Heuristics for"
        " Classical Planning",
        "https://ai.dmi.unibas.ch/papers/sievers-et-al-socs2012.pdf",
        "Proceedings of the Fifth Annual Symposium on Combinatorial"
        " Search (SoCS 2012)",
        "105-111",
        "AAAI Press",
        "2012");
}
class CompressedPDBHeuristicFeature
    : public plugins::TypedFeature<Evaluator, CompressedPDBHeuristic> {
public:
    CompressedPDBHeuristicFeature() : TypedFeature("compressed_pdb") {
        document_subcategory("heuristics_pdb");
        document_title("Pattern database heuristic");
        document_synopsis(
            "Computes goal distance in "
            "state space abstractions based on projections. "
            "First used in domain-independent planning by:"
            + paper_references());

        add_option<shared_ptr<PatternGenerator>>(
            "pattern",
            "pattern generation method",
            "greedy()");
        add_heuristic_options_to_feature(*this, "compressed_pdb");

        document_language_support("action costs", "supported");
        document_language_support("conditional effects", "not supported");
        document_language_support("axioms", "not supported");

        document_property("admissible", "yes");
        document_property("consistent", "yes");
        document_property("safe", "yes");
        document_property("preferred operators", "no");
    }

    virtual shared_ptr<CompressedPDBHeuristic>
    create_component(const plugins::Options &opts) const override {
        return plugins::make_shared_from_arg_tuples<CompressedPDBHeuristic>(
            opts.get<shared_ptr<PatternGenerator>>("pattern"),
            get_heuristic_arguments_from_options(opts)
            );
    }
};

static plugins::FeaturePlugin<CompressedPDBHeuristicFeature> _plugin;
}
