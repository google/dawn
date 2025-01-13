import subprocess
import re
import random
import numpy as np
from scipy.optimize import differential_evolution

np.set_printoptions(precision=0, suppress=True, formatter={'float_kind': lambda x: str(int(x))})

def print_stats(xk, convergence):
    print(f"Current best solution: {xk} convergence measure: {convergence:.6f}")

def fn(args):
    args = [
        "./tint_structure_fuzzer",
        "--prob="+",".join(str(int(x)) for x in args),
        "-cross_over=0", 
        "-mutate_depth=1", 
        "-max_total_time=30",
        "-print_funcs=0",
        "-fsanitize-coverage-ignorelist=./blocklist.txt"
    ]
    print(f"Calling {args}...")
    result = subprocess.run(args, stderr=subprocess.PIPE, stdout=subprocess.PIPE)
    out = str(result.stderr)
    matches = re.findall(r"cov: (\d+)", out)
    if not matches:
        raise ValueError("Coverage result not found")
    result = int(matches[-1])
    print(f"Coverage = {result}")
    return -result

print("tint_structure_fuzzer optimizer")
val = differential_evolution(fn, bounds=[(0, 1000)] * 8, maxiter=100, popsize=10, callback=print_stats)
print(f"Best solution is {val.x} -> {-val.fun}")
