var<private> u : vec3<i32> = vec3<i32>(vec3<f32>(1.0f));

@compute @workgroup_size(1)
fn main() {
  _ = u;
}
