var<private> u : vec2<f32> = vec2<f32>(vec2<u32>(1u));

@compute @workgroup_size(1)
fn main() {
  _ = u;
}
