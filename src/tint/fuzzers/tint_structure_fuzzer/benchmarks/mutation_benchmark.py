
import subprocess
import re
import time
import numpy as np
import matplotlib.pyplot as plt
import argparse
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

def run_fuzzer_with_mutation(mutation_idx, runtime_seconds, weight=100):
    """Run the fuzzer with a specific mutation type emphasized"""

    probs = [1] * len(MUTATIONS)
    probs[mutation_idx] = weight  
    
    cmd_args = [
        "./tint_structure_fuzzer",
        f"--prob={','.join(map(str, probs))}",
        "-cross_over=0",
        "-mutate_depth=1",
        f"-max_total_time={runtime_seconds}",
        "-print_funcs=0",
        "-jobs=1"
    ]
    
    print(f"Running fuzzer with {MUTATIONS[mutation_idx]} mutation for {runtime_seconds} seconds...")
    

    result = subprocess.run(cmd_args, stderr=subprocess.PIPE, stdout=subprocess.PIPE)
    output = result.stderr.decode('utf-8')
    

    cov_pattern = r"cov: (\d+)"
    coverage_values = re.findall(cov_pattern, output)
    
    if not coverage_values:
        print("Warning: No coverage data found")
        return [], []
    

    time_pattern = r"ft: (\d+)"
    timestamps = re.findall(time_pattern, output)
    
    if len(timestamps) != len(coverage_values):

        timestamps = list(np.linspace(0, runtime_seconds, len(coverage_values)))
    else:

        timestamps = [float(t)/1000 for t in timestamps]
        
    coverage_values = [int(c) for c in coverage_values]
    
    return timestamps, coverage_values

def run_benchmark(runtime_per_mutation, repeats=3):
    """Run benchmark for all mutation types"""
    results = {}
    
    for idx, mutation in enumerate(MUTATIONS):
        print(f"\n=== Benchmarking {mutation} ===")
        all_timestamps = []
        all_coverages = []
        
        for i in range(repeats):
            print(f"Run {i+1}/{repeats}")
            timestamps, coverages = run_fuzzer_with_mutation(idx, runtime_per_mutation)
            if timestamps and coverages:
                all_timestamps.append(timestamps)
                all_coverages.append(coverages)
        
        if all_timestamps and all_coverages:
            results[mutation] = {
                'timestamps': all_timestamps,
                'coverages': all_coverages
            }
    
    return results

def plot_results(results, output_dir):
    """Generate plots from benchmark results"""
    Path(output_dir).mkdir(exist_ok=True)
    

    plt.figure(figsize=(12, 8))
    
    for mutation, data in results.items():

        avg_coverage = []
        for run_coverages in data['coverages']:

            if run_coverages:
                initial = run_coverages[0]
                normalized = [(c - initial) / initial * 100 for c in run_coverages]
                avg_coverage.append(normalized)
        
        if not avg_coverage:
            continue
            

        timestamps = data['timestamps'][0]
        

        coverage_array = np.array([c[:len(timestamps)] for c in avg_coverage if len(c) >= len(timestamps)])
        if len(coverage_array) == 0:
            continue
            
        mean_coverage = np.mean(coverage_array, axis=0)
        std_coverage = np.std(coverage_array, axis=0)
        

        plt.plot(timestamps, mean_coverage, label=mutation)
        plt.fill_between(timestamps, 
                         mean_coverage - std_coverage, 
                         mean_coverage + std_coverage, 
                         alpha=0.2)
    
    plt.title('Coverage Growth Rate by Mutation Type')
    plt.xlabel('Time (seconds)')
    plt.ylabel('Coverage Growth (%)')
    plt.legend()
    plt.grid(True)
    plt.savefig(f"{output_dir}/coverage_over_time.png")
    

    plt.figure(figsize=(12, 6))
    
    final_coverages = []
    labels = []
    errors = []
    
    for mutation, data in results.items():
        final_values = []
        for run_coverages in data['coverages']:
            if run_coverages:

                initial = run_coverages[0]
                final = run_coverages[-1]
                pct_increase = (final - initial) / initial * 100
                final_values.append(pct_increase)
        
        if final_values:
            labels.append(mutation)
            final_coverages.append(np.mean(final_values))
            errors.append(np.std(final_values))
    
    plt.bar(labels, final_coverages, yerr=errors)
    plt.title('Final Coverage Improvement by Mutation Type')
    plt.xlabel('Mutation Type')
    plt.ylabel('Coverage Improvement (%)')
    plt.xticks(rotation=45)
    plt.tight_layout()
    plt.savefig(f"{output_dir}/final_coverage_comparison.png")
    

    data = []
    for mutation, result in results.items():
        for run_idx, (timestamps, coverages) in enumerate(zip(result['timestamps'], result['coverages'])):
            for t, c in zip(timestamps, coverages):
                data.append({
                    'mutation': mutation,
                    'run': run_idx + 1,
                    'time': t,
                    'coverage': c
                })
    
    df = pd.DataFrame(data)
    df.to_csv(f"{output_dir}/mutation_benchmark_results.csv", index=False)

def main():
    parser = argparse.ArgumentParser(description='Benchmark different mutation types for the WGSL fuzzer')
    parser.add_argument('--time', type=int, default=10, 
                        help='Runtime in seconds per mutation type (default: 60)')
    parser.add_argument('--repeats', type=int, default=3,
                        help='Number of repetitions per mutation type (default: 3)')
    parser.add_argument('--output', type=str, default='benchmark_results',
                        help='Output directory for results (default: benchmark_results)')
    
    args = parser.parse_args()
    
    print(f"Starting mutation benchmark with {args.time}s per mutation, {args.repeats} repeats")
    
    results = run_benchmark(args.time, args.repeats)
    plot_results(results, args.output)
    
    print(f"Benchmark complete. Results saved to {args.output}/")

if __name__ == "__main__":
    main() 