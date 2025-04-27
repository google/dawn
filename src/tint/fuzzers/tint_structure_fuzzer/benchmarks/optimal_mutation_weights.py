#!/usr/bin/env python3
import subprocess
import re
import numpy as np
import argparse
import pandas as pd
from pathlib import Path
import matplotlib.pyplot as plt
from datetime import datetime


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

class Particle:
    def __init__(self, bounds):
        self.position = np.array([np.random.uniform(low, high) for low, high in bounds])
        self.velocity = np.array([np.random.uniform(-0.5, 0.5) for _ in bounds])
        self.best_position = self.position.copy()
        self.best_score = float('-inf')  # We're maximizing coverage

class PSO:
    def __init__(self, objective_func, bounds, num_particles=10, max_iter=30):
        self.objective_func = objective_func
        self.bounds = bounds
        self.num_particles = num_particles
        self.max_iter = max_iter
        

        self.w = 0.7  
        self.c1 = 1.5 
        self.c2 = 1.5  
        

        self.particles = [Particle(bounds) for _ in range(num_particles)]
        self.global_best_position = None
        self.global_best_score = float('-inf')
        

        self.history = []
    
    def optimize(self):
        for iteration in range(self.max_iter):
            print(f"\nIteration {iteration+1}/{self.max_iter}")
            
            for i, particle in enumerate(self.particles):

                print(f"Evaluating particle {i+1}/{self.num_particles}")
                score = self.objective_func(particle.position)
                

                if score > particle.best_score:
                    particle.best_score = score
                    particle.best_position = particle.position.copy()
                

                if score > self.global_best_score:
                    self.global_best_score = score
                    self.global_best_position = particle.position.copy()
                    print(f"New best score: {score}")
                    print(f"New best position: {self.global_best_position}")
            

            self.history.append({
                'iteration': iteration,
                'best_score': self.global_best_score,
                'best_position': self.global_best_position.copy()
            })
            

            for particle in self.particles:

                r1 = np.random.random(len(self.bounds))
                r2 = np.random.random(len(self.bounds))
                

                cognitive = self.c1 * r1 * (particle.best_position - particle.position)
                social = self.c2 * r2 * (self.global_best_position - particle.position)
                particle.velocity = self.w * particle.velocity + cognitive + social
                

                particle.position = particle.position + particle.velocity
                

                for j in range(len(self.bounds)):
                    particle.position[j] = np.clip(
                        particle.position[j], 
                        self.bounds[j][0], 
                        self.bounds[j][1]
                    )
        

        return self.global_best_position, self.global_best_score, self.history

def run_fuzzer_with_weights(weights, runtime_seconds):
    """Run the fuzzer with specified weights for each mutation type and return coverage"""

    weights = np.round(np.maximum(weights, 1)).astype(int)
    
    cmd_args = [
        "./tint_structure_fuzzer",
        f"--prob={','.join(map(str, weights))}",
        "-cross_over=0",
        "-mutate_depth=1",
        f"-max_total_time={runtime_seconds}",
        "-print_funcs=0",
        "-jobs=1"
    ]
    
    print(f"Running fuzzer with weights: {weights}")
    

    result = subprocess.run(cmd_args, stderr=subprocess.PIPE, stdout=subprocess.PIPE)
    output = result.stderr.decode('utf-8')
    

    cov_pattern = r"cov: (\d+)"
    coverage_values = re.findall(cov_pattern, output)
    
    if not coverage_values:
        print("Warning: No coverage data found")
        return 0
    
    final_coverage = int(coverage_values[-1])
    print(f"Final coverage: {final_coverage}")
    
    return final_coverage

def plot_optimization_progress(history, output_dir):
    """Plot the optimization progress over iterations"""
    Path(output_dir).mkdir(exist_ok=True)
    

    iterations = [entry['iteration'] for entry in history]
    scores = [entry['best_score'] for entry in history]
    

    plt.figure(figsize=(10, 6))
    plt.plot(iterations, scores, marker='o')
    plt.title('PSO Optimization Progress')
    plt.xlabel('Iteration')
    plt.ylabel('Best Coverage')
    plt.grid(True)
    plt.savefig(f"{output_dir}/optimization_progress.png")
    

    final_weights = history[-1]['best_position']
    
    plt.figure(figsize=(12, 6))
    plt.bar(MUTATIONS, final_weights)
    plt.title('Optimal Mutation Weights')
    plt.xlabel('Mutation Type')
    plt.ylabel('Weight')
    plt.xticks(rotation=45)
    plt.tight_layout()
    plt.savefig(f"{output_dir}/optimal_weights.png")
    

    history_data = []
    for entry in history:
        row = {
            'iteration': entry['iteration'],
            'best_score': entry['best_score']
        }
        for i, mutation in enumerate(MUTATIONS):
            row[mutation] = entry['best_position'][i]
        history_data.append(row)
    
    df = pd.DataFrame(history_data)
    df.to_csv(f"{output_dir}/optimization_history.csv", index=False)

def main():
    parser = argparse.ArgumentParser(description='Find optimal mutation weights using PSO')
    parser.add_argument('--time', type=int, default=30, 
                        help='Runtime in seconds per evaluation (default: 30)')
    parser.add_argument('--particles', type=int, default=5,
                        help='Number of particles in PSO (default: 5)')
    parser.add_argument('--iterations', type=int, default=10,
                        help='Number of PSO iterations (default: 10)')
    parser.add_argument('--output', type=str, default='pso_results',
                        help='Output directory for results (default: pso_results)')
    
    args = parser.parse_args()
    
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    output_dir = f"{args.output}_{timestamp}"
    
    print(f"Starting PSO optimization with {args.particles} particles and {args.iterations} iterations")
    

    def objective(weights):
        return run_fuzzer_with_weights(weights, args.time)
    

    bounds = [(1, 100)] * len(MUTATIONS)
    

    pso = PSO(
        objective_func=objective,
        bounds=bounds,
        num_particles=args.particles,
        max_iter=args.iterations
    )
    
    best_position, best_score, history = pso.optimize()
    
    print("\nOptimization Complete!")
    print(f"Best Coverage: {best_score}")
    print("Best Weights:")
    for i, mutation in enumerate(MUTATIONS):
        print(f"  {mutation}: {int(best_position[i])}")
    

    plot_optimization_progress(history, output_dir)
    
    print(f"\nResults saved to {output_dir}/")
    

    print("\nRunning final validation with best weights...")
    final_coverage = run_fuzzer_with_weights(best_position, args.time * 2)
    print(f"Final validation coverage: {final_coverage}")

if __name__ == "__main__":
    main() 