var<private> u = vec3<f32>(1.0f);

@compute @workgroup_size(1)
fn f() {
  let v : vec3<bool> = vec3<bool>(u);
}
