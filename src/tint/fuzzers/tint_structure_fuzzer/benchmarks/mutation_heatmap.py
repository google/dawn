#!/usr/bin/env python3
import subprocess
import re
import numpy as np
import matplotlib.pyplot as plt
import argparse
import seaborn as sns
from pathlib import Path
import pandas as pd


MUTATIONS = [
    "AddOptional",
    "RemoveOptional", 
    "IncRepeat",
    "DecRepeat",
    "NextAlternative",
    "PrevAlternative", 
    "RandomAlternative",
    "RandomTerminal",
    "SubtreeTransfer"
]

def run_fuzzer_with_mutation_pair(mutation1_idx, mutation2_idx, runtime_seconds, weight=50):
   
    probs = [1] * len(MUTATIONS)
    probs[mutation1_idx] = weight
    probs[mutation2_idx] = weight
    
    cmd_args = [
        "./tint_structure_fuzzer",
        f"--prob={','.join(map(str, probs))}",
        "-cross_over=0",
        "-mutate_depth=1",
        f"-max_total_time={runtime_seconds}",
        "-print_funcs=0",
        "-jobs=1"
    ]
    
    print(f"Running fuzzer with {MUTATIONS[mutation1_idx]} + {MUTATIONS[mutation2_idx]} for {runtime_seconds} seconds...")
    

    result = subprocess.run(cmd_args, stderr=subprocess.PIPE, stdout=subprocess.PIPE)
    output = result.stderr.decode('utf-8')
    

    cov_pattern = r"cov: (\d+)"
    coverage_values = re.findall(cov_pattern, output)
    
    if not coverage_values:
        print("Warning: No coverage data found")
        return 0
    

    return int(coverage_values[-1])

def generate_mutation_heatmap(runtime_per_pair, output_dir):
    """Generate a heatmap showing effectiveness of mutation pairs"""
    Path(output_dir).mkdir(exist_ok=True)
    

    n_mutations = len(MUTATIONS)
    coverage_matrix = np.zeros((n_mutations, n_mutations))
    

    for i in range(n_mutations):
        for j in range(i, n_mutations):
            coverage = run_fuzzer_with_mutation_pair(i, j, runtime_per_pair)
            coverage_matrix[i, j] = coverage
            if i != j: 
                coverage_matrix[j, i] = coverage
    

    max_coverage = np.max(coverage_matrix)
    if max_coverage > 0:
        relative_matrix = (coverage_matrix / max_coverage) * 100
    else:
        relative_matrix = coverage_matrix
    

    plt.figure(figsize=(12, 10))
    sns.heatmap(relative_matrix, annot=True, fmt=".1f", 
                xticklabels=MUTATIONS, yticklabels=MUTATIONS, 
                cmap="viridis")
    plt.title('Относительная эффективность пар мутаций (%)')
    plt.tight_layout()
    plt.savefig(f"{output_dir}/mutation_pair_heatmap.png")
    

    df = pd.DataFrame(coverage_matrix, index=MUTATIONS, columns=MUTATIONS)
    df.to_csv(f"{output_dir}/mutation_pair_coverage.csv")
    

    plt.figure(figsize=(12, 10))
    sns.heatmap(coverage_matrix, annot=True, fmt=".0f", 
                xticklabels=MUTATIONS, yticklabels=MUTATIONS, 
                cmap="viridis")
    plt.title('Absolute Coverage of Mutation Pairs')
    plt.tight_layout()
    plt.savefig(f"{output_dir}/mutation_pair_heatmap_absolute.png")
    
    return coverage_matrix

def main():
    parser = argparse.ArgumentParser(description='Generate a heatmap of mutation pair effectiveness')
    parser.add_argument('--time', type=int, default=10, 
                        help='Runtime in seconds per mutation pair (default: 30)')
    parser.add_argument('--output', type=str, default='mutation_heatmap',
                        help='Output directory for results (default: mutation_heatmap)')
    
    args = parser.parse_args()
    
    print(f"Starting mutation pair benchmark with {args.time}s per pair")
    
    coverage_matrix = generate_mutation_heatmap(args.time, args.output)
    
    
    n_mutations = len(MUTATIONS)
    best_pairs = []
    
    for i in range(n_mutations):
        for j in range(i+1, n_mutations):  
            best_pairs.append((MUTATIONS[i], MUTATIONS[j], coverage_matrix[i, j]))
    

    best_pairs.sort(key=lambda x: x[2], reverse=True)
    
    print("\nTop 5 Most Effective Mutation Pairs:")
    for i, (mut1, mut2, cov) in enumerate(best_pairs[:5]):
        print(f"{i+1}. {mut1} + {mut2}: {cov:.0f}")
    
    print(f"\nBenchmark complete. Results saved to {args.output}/")

if __name__ == "__main__":
    main() 