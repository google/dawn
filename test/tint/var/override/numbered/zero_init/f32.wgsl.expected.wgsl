const o : f32 = 0.0f;

@compute @workgroup_size(1)
fn main() {
  if ((o == 0.0)) {
    _ = o;
  }
}
