var<private> u : i32 = i32(f32(1.0f));

@compute @workgroup_size(1)
fn main() {
  _ = u;
}
