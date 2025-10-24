var<private> u : vec4<f32> = vec4<f32>(vec4<u32>(1u));

@compute @workgroup_size(1)
fn main() {
  _ = u;
}
