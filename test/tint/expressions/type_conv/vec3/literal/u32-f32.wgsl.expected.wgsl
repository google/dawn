var<private> u : vec3<f32> = vec3<f32>(vec3<u32>(1u));

@compute @workgroup_size(1)
fn main() {
  _ = u;
}
