override o : f32 = 1.0;

@compute @workgroup_size(1)
fn main() {
  _ = o;
}
