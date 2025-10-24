@group(0) @binding(0) var<storage, read_write> a : vec4<u32>;

@compute @workgroup_size(1)
fn main() {
  a[1]++;
  a.z++;
}
