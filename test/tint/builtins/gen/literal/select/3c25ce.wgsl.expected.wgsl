fn select_3c25ce() {
  var res : vec3<bool> = select(vec3<bool>(true), vec3<bool>(true), true);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_3c25ce();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_3c25ce();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_3c25ce();
}
