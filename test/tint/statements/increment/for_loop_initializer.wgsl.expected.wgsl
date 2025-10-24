@group(0) @binding(0) var<storage, read_write> i : u32;

@compute @workgroup_size(1)
fn main() {
  for(i++; (i < 10u); ) {
  }
}
