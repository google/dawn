fn select_78be5f() {
  var res : vec3<f32> = select(vec3<f32>(1.0f), vec3<f32>(1.0f), true);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_78be5f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_78be5f();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_78be5f();
}
