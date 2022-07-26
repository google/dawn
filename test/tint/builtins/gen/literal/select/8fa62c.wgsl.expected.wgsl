fn select_8fa62c() {
  var res : vec3<i32> = select(vec3<i32>(1), vec3<i32>(1), true);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_8fa62c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_8fa62c();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_8fa62c();
}
