#!/usr/bin/env python3

from itertools import product
import os, sys
from pathlib import Path
from subprocess import check_call

DOWNWARD = Path(os.environ["DOWNWARD_REPO"]) / "fast-downward.py"

class Task:
    def __init__(self, variables, operators, mutex_groups) -> None:
        self.variables = variables
        self.operators = operators
        self.mutex_with = dict()
        for mutex in mutex_groups:
            for atom in mutex:
                if atom not in self.mutex_with:
                    self.mutex_with[atom] = set()
                self.mutex_with[atom] |= set(x for x in mutex if x != atom)

class Operator:
    def __init__(self, stream) -> None:
        self.name = ""
        self.precondition = dict()
        self.effect = dict()
        self.cost = 0

        # Read name
        self.name = stream.readline().strip()

        # Read prevail conditions
        num_prevails = int(stream.readline())
        for _ in range(num_prevails):
            var, val = stream.readline().split()
            self.precondition[var] = val

        # Read pre/post effects
        num_effects = int(stream.readline())
        for _ in range(num_effects):
            prepost = [int(x) for x in stream.readline().split()]
            if len(prepost) != 4:
                print("Task has conditional effects")
                sys.exit(1)
            n_cond, var, pre, post = prepost
            assert n_cond == 0

            if pre != -1:
                self.precondition[var] = pre
            self.effect[var] = post
        
        # Read cost
        self.cost = int(stream.readline())

        assert stream.readline().strip() == "end_operator"

    def get_prevail_var_ids(self):
        return set(self.effect) - set(self.precondition)

    def get_post_condition(self):
        return self.precondition | self.effect


class Variable:
    def __init__(self, varid, stream) -> None:
        self.name = ""
        self.varid = varid
        self.values = []

        # Read name
        self.name = stream.readline()

        axiom_layer = int(stream.readline())
        if axiom_layer != -1:
            print("Task has axioms")
            sys.exit(1)
        
        # Read values
        num_values = int(stream.readline())
        for _ in range(num_values):
            self.values.append(stream.readline().strip())

        assert stream.readline().strip() == "end_variable"


def violates_mutex(state: dict[int, int], task: Task):
    for atom1 in state.items():
        for atom2 in state.items():
            if atom2 in task.mutex_with.get(atom1, []):
                return True
    return False

#get all partial states to which a set of inverse operators have to reach for invertibility of o
#only the variables in eff(o) but not in pre(o) are relevant
def get_partial_states(variables: list[Variable]):
    facts_per_variable = [[(v.varid, i) for i in range(len(v.values))] for v in variables]
    return (dict(state) for state in product(*facts_per_variable))

# Pretty-printing for (partial) states
def translate(state, task):
    return [task.variables[v].values[d] for v,d in state.items()]

# Check if candidate is the inverse of op, given the context
def is_inverse(op: Operator, candidate: Operator, context: dict[int, int], task):
    post = op.get_post_condition()
    
    # Check if op results in a state where candidate is applicable
    for v, d in candidate.precondition.items():
        if v not in post or post[v] != d:
            return False

    # Check if candidate restores all changed values to their original values
    original_values = context | op.precondition
    post_candidate = post | candidate.effect
    for v in op.effect | candidate.effect:
        if post_candidate.get(v) != original_values.get(v):
            return False

    print(f"Operator '{candidate.name}' is the inverse of '{op.name}' with context {translate(context, task)}")
    return True

#check if an operator is invertible
def has_inverse(op: Operator, task: Task):
    prevail_vars = [task.variables[v] for v in op.get_prevail_var_ids()]
    for context in get_partial_states(prevail_vars):
        if violates_mutex(context | op.precondition, task):
#            print(f"Skipping context {translate(context, task)} for operator '{op.name}' because it violates a mutex")
            continue
        for candidate in task.operators:
#            print(f"checking invertibility for '{op.name}' with '{candidate.name}' in context {translate(context, task)}.")
            if is_inverse(op, candidate, context, task):
                break
        else:
            print(f"Operator '{op.name}' has no inverse for context {translate(context, task)}")
            return False
    return True


# Check each operator in task for invertibility
def is_invertible(task: Task):
    for op in task.operators:
        if not has_inverse(op, task):
            return False
    return True


def parse_mutex_group(stream):
    entries = []
    num_entries = int(stream.readline())
    for _ in range(num_entries):
        var, val = stream.readline().strip().split()
        entries.append((int(var), int(val)))

    assert stream.readline().strip() == "end_mutex_group"
    return entries


def read_task(filename):
    variables = []
    operators = []
    mutex_groups = []
    var_id = 0
    with open(filename, "rt") as sas:
        for line in sas:
            if line == "begin_variable\n":
                variables.append(Variable(var_id, sas))
                var_id += 1
            elif line == "begin_operator\n":
                operators.append(Operator(sas))
            elif line == "begin_mutex_group\n":
                mutex_groups.append(parse_mutex_group(sas))
    return Task(variables, operators, mutex_groups)

def translate_pddl_file(filename):
    cmd = [DOWNWARD, "--translate", filename]
    check_call(cmd)

def is_sas_task_invertible(filename):
    return is_invertible(read_task(filename))

def is_pddl_domain_invertible(domain_dir: Path):
    for pddl_file in domain_dir.iterdir():
        if "domain" not in pddl_file.name:
            translate_pddl_file(pddl_file)
            if not is_sas_task_invertible("output.sas"):
                print(f"{pddl_file} is not invertible")
                return False
    return True

def main():
    if is_pddl_domain_invertible(Path(sys.argv[1])):
        print("All tasks in the domain are invertible")

if __name__ == "__main__":
    main()