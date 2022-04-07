SKIP: FAILED


@group(0) @binding(0) var<storage, read_write> i : u32;

fn main() {
  for(; (i < 10u); i--) {
  }
}

Failed to generate: error: cannot modify value of type 'u32'
