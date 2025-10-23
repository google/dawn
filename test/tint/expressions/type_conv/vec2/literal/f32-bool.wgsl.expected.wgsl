var<private> u : vec2<bool> = vec2<bool>(vec2<f32>(1.0f));

@compute @workgroup_size(1)
fn main() {
  _ = u;
}
