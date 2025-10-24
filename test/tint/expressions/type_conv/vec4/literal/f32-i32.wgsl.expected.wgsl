var<private> u : vec4<i32> = vec4<i32>(vec4<f32>(1.0f));

@compute @workgroup_size(1)
fn main() {
  _ = u;
}
