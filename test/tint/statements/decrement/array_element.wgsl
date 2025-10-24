@group(0) @binding(0) var<storage, read_write> a : array<u32>;

@compute @workgroup_size(1)
fn main() {
  a[1]--;
}
