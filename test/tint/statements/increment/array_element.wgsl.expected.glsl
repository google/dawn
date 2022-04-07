SKIP: FAILED


@group(0) @binding(0) var<storage, read_write> a : array<u32>;

fn main() {
  a[1]++;
}

Failed to generate: error: unknown statement type: tint::ast::IncrementDecrementStatement
