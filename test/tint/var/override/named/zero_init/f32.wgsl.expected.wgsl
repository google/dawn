override o : f32 = f32();

@compute @workgroup_size(1)
fn main() {
  _ = o;
}
