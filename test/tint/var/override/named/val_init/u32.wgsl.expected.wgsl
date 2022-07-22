const o : u32 = 0u;

@compute @workgroup_size(1)
fn main() {
  if ((o == 2)) {
    _ = o;
  }
}
