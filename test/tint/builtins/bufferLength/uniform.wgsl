@group(0) @binding(0) var<uniform> v : buffer<64>;

@group(0) @binding(1) var<storage, read_write> out : u32;

@compute @workgroup_size(1)
fn main() {
  out = bufferLength(&v);
}

