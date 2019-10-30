#!/usr/bin/env python

"""
Generates rules of the form A --> A B. The paramter 'power' determiens the
number of rules that will be generated. The minimum number is 100,as the base
set has 10 elements and every rule requires at least a left and a right side.
Therefor the minmum number is 10*10 rules and the minimum value of 'power' to
actually generate rules is 2. 10^'power' rules will be generated.
All rules are unique and set of all rules is the complete set of permutations
of (A-J)^'power'.
"""

from sys import argv, stderr

upper_chars = [chr(item).upper() for item in range(ord('a'), ord('j')+1)]


def gen_rhs(power, set):
    rhss = list()
    if power == 0:
        return set
    if power > 0:
        for c in set:
            for s in gen_rhs(power-1, set):
                rhss.append(c+" "+s)
    return rhss


def gen_rules(power, set, file):
    cnt = 0
    with open(file, "w") as f:
        for lhs in set:
            for rhs in gen_rhs(power-1, set):
                f.write(lhs+" --> "+rhs+"\n")
                cnt+=1
	f.write("S --> A\n")
    print(str(cnt)+" rules generated")


if __name__ == '__main__':
    if len(argv) == 3:
        gen_rules(int(argv[1])-1, upper_chars, argv[2])
    else:
        stderr.write("usage: power, file\n")
