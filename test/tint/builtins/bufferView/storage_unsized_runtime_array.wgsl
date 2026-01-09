@group(0) @binding(0) var<storage, read_write> v : buffer;

@compute @workgroup_size(1)
fn main() {
  let p = bufferView<array<u32>>(&v, 0);
  p[4] = 4;
}
