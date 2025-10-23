var<private> u : f32 = f32(bool(true));

@compute @workgroup_size(1)
fn main() {
  _ = u;
}
