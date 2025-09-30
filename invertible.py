#!/usr/bin/env python3

from itertools import product
import os


def read_variable(sas, var_domains, var_counter):
    for i in range(0, 2):
        sas.readline()
    var_domains[var_counter] = int(sas.readline())

def read_operator(sas, op_pre, op_eff, op_v_zero, op_counter):
    pre = dict()    #dict of variable id -> value
    eff = dict()    #dict of variable id -> value
    v_zero = []     #list of variable ids

    #read prevail conditions
    sas.readline()
    n_prevail = int(sas.readline())
    for i in range(0, n_prevail):
        arg = sas.readline().split()
        pre[int(arg[0])] = int(arg[1])     #variable id -> value

    #read effects
    n_effect = int(sas.readline())
    for i in range(0, n_effect):
        arg = sas.readline().split()
        op_id = int(arg[1])
        pre_value = int(arg[2])
        post_value = int(arg[3])

        if pre_value == -1:
            v_zero.append(op_id)        #variable in effect but not in precondition
        else:
            pre[op_id] = pre_value      #precondition

        eff[op_id] = post_value

    op_pre[op_counter] = pre
    op_eff[op_counter] = eff
    op_v_zero[op_counter] = v_zero

def get_post_condition(op_id, op_pre, op_eff):
    post_o = op_pre[op_id].copy()
    for v in op_eff[op_id]:
        post_o[v] = op_eff[op_id][v]
    return post_o

#combine two partial states by copying the first one and then writing the second one over the first one
def combine_dicts_in_order(d_one, d_two):
    result = d_one.copy()
    for v in d_two:
        result[v] = d_two[v]
    return result

def construct_partial_states(partial_states, partial_state, v_zero, var_domains, count):
    if count >= len(v_zero):
        partial_states.append(partial_state)
        return
    for i in range(0, var_domains[v_zero[count]]):
        current = partial_state.copy()
        current[v_zero[count]] = i
        construct_partial_states(partial_states, current, v_zero, var_domains, count + 1)

#get all partial states to which a set of inverse operators have to reach for invertibility of o
#only the variables in eff(o) but not in pre(o) are relevant
def get_partial_states(v_zero, var_domains):
    partial_states = []
    if len(v_zero) == 0:
        partial_states.append(dict())
        return partial_states

    construct_partial_states(partial_states, dict(), v_zero, var_domains, 0)
    return partial_states

#check if o_inv is the inverse of op_id, given the context
def is_inverse(op_id, o_inv, context, op_pre, op_eff):
    post_o = get_post_condition(op_id, op_pre, op_eff)
    
    #check if o results in a state where o_inv is applicable
    for v in op_pre[o_inv]:
        if v not in post_o or post_o[v] != op_pre[o_inv][v]:
            return False

    #check if post(o_inv) results in before(o)
    post_o_inv = combine_dicts_in_order(post_o, op_eff[o_inv])
    before_o = combine_dicts_in_order(op_pre[op_id], context)
    if len(post_o_inv) != len(before_o):
        return False
    for v in before_o:
        if post_o_inv[v] != before_o[v]:
            return False

    #print("the operator", o_inv, "is the inverse of", op_id, "with context", context)
    return True

#check if an operator is invertible
def has_inverse(op_id, op_pre, op_eff, op_v_zero, op_count, var_domains):
    partial_states = get_partial_states(op_v_zero[op_id], var_domains)
    for context in partial_states:
        for o_inv in range(0, op_count):
            #print("checking invertibility for o=", op_id, " and o_inv=", o_inv)
            if is_inverse(op_id, o_inv, context, op_pre, op_eff):
                break
        else:
            print("operator", op_id, "has no inverse for context", context)
            return False
    return True

#check each operator in domain for invertibility
def is_invertible(op_count, op_pre, op_eff, op_v_zero, var_domains):
    for i in range(0, op_count):
        if not has_inverse(i, op_pre, op_eff, op_v_zero, op_count, var_domains):
            return False
    return True


def main():
    #read the sas+ file and extract information about variables and operators
    sas = open("output.sas", "rt")

    var_domains = dict()
    var_counter = 0
    op_pre = dict()
    op_eff = dict()
    op_v_zero = dict()
    op_counter = 0
    
    for line in sas:
        if line == "begin_variable\n":
            read_variable(sas, var_domains, var_counter)
            var_counter += 1
        elif line == "begin_operator\n":
            read_operator(sas, op_pre, op_eff, op_v_zero, op_counter)
            op_counter += 1

    sas.close()

    #check invertibility of entire domain
    if is_invertible(op_counter, op_pre, op_eff, op_v_zero, var_domains):
        print("the domain is invertible.")
    else:
        print("the domain is NOT invertible.")


if __name__ == "__main__":
    main()