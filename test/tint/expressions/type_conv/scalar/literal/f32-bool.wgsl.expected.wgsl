var<private> u : bool = bool(f32(1.0f));

@compute @workgroup_size(1)
fn main() {
  _ = u;
}
