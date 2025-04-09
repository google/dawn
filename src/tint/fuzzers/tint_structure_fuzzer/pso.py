import subprocess
import re
import random
import numpy as np
import argparse

class Particle:
    def __init__(self, bounds):
        self.position = np.array([random.uniform(low, high) for low, high in bounds])
        self.position[self.position == 0] = random.uniform(0.01, 1) 
        self.velocity = np.array([random.uniform(-1, 1) for _ in bounds])
        self.best_position = self.position.copy()
        self.best_score = float('inf')

class PSO:
    def __init__(self, objective_func, bounds, num_particles=10, max_iter=100):
        self.objective_func = objective_func
        self.bounds = bounds
        self.num_particles = num_particles
        self.max_iter = max_iter
        
        # PSO parameters
        self.w = 0.7  
        self.c1 = 2.0  
        self.c2 = 2.0  
        self.particles = [Particle(bounds) for _ in range(num_particles)]
        self.global_best_position = None
        self.global_best_score = float('inf')
        
    def optimize(self, callback=None):
        best_coverage_per_iteration = []
        
        for iteration in range(self.max_iter):
            iteration_best_score = float('inf')
            
            for particle in self.particles:
                score = self.objective_func(particle.position)
                
                # Update personal best
                if score < particle.best_score:
                    particle.best_score = score
                    particle.best_position = particle.position.copy()
                    
                if score < self.global_best_score:
                    self.global_best_score = score
                    self.global_best_position = particle.position.copy()
            
            iteration_best_score = -self.global_best_score  # Since we're maximizing the coverage
            best_coverage_per_iteration.append(iteration_best_score)
            
            for particle in self.particles:
                r1, r2 = random.random(), random.random()
                
                cognitive = self.c1 * r1 * (particle.best_position - particle.position)
                social = self.c2 * r2 * (self.global_best_position - particle.position)
                particle.velocity = (self.w * particle.velocity + cognitive + social)
                
                particle.position = particle.position + particle.velocity
                
                particle.position = np.clip(particle.position, 
                                         [b[0] for b in self.bounds],
                                         [b[1] for b in self.bounds])
                
                particle.position[particle.position == 0] = random.uniform(0.01, 1)
            
            if callback:
                callback(self.global_best_position, iteration)
            
            print(f"Iteration {iteration}: Best coverage = {iteration_best_score}")
        
        return self.global_best_position, self.global_best_score

def print_stats(xk, iteration):
    print(f"Iteration {iteration}: Current best solution: {xk}")

def fn(args, max_time):
    args = np.round(args).astype(int)
    args[args == 0] = random.randint(1, 1000)  # Replace zeros with a random value

    cmd_args = [
        "./tint_structure_fuzzer",
        "--prob=" + ",".join(str(x) for x in args),
        "-cross_over=0",
        "-mutate_depth=1",
        f"-max_total_time={max_time}",
        "-print_funcs=0",
        "-fsanitize-coverage-ignorelist=./blocklist.txt",
        "-jobs=1"
    ]
    print(f"Calling {cmd_args}...")
    
    result = subprocess.run(cmd_args, stderr=subprocess.PIPE, stdout=subprocess.PIPE)
    out = str(result.stderr)
    matches = re.findall(r"cov: (\d+)", out)
    
    if not matches:
        raise ValueError("Coverage result not found")
        
    result = int(matches[-1])
    print(f"Coverage = {result}")
    return -result

def main():
    parser = argparse.ArgumentParser(description='Tint Structure Fuzzer Optimizer using PSO')
    parser.add_argument('--max_time', type=int, default=120,
                       help='Maximum total time for each fuzzing run (default: 120)')
    args = parser.parse_args()
    
    print("tint_structure_fuzzer optimizer (PSO)")
    
    # Define bounds for 8 parameters
    bounds = [(0, 1000)] * 8
    
    # Create PSO optimizer
    pso = PSO(
        objective_func=lambda x: fn(x, args.max_time),
        bounds=bounds,
        num_particles=10,
        max_iter=100
    )
    
    best_position, best_score = pso.optimize(callback=print_stats)
    
    print(f"Best solution is {best_position} -> {-best_score}")

if __name__ == "__main__":
    main()


