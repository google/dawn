SKIP: FAILED


@group(0) @binding(0) var<storage, read_write> a : vec4<u32>;

fn main() {
  a[1]--;
  a.z--;
}

Failed to generate: error: cannot modify value of type 'u32'
