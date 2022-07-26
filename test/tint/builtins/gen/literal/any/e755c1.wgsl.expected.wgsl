fn any_e755c1() {
  var res : bool = any(vec3<bool>(true));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  any_e755c1();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  any_e755c1();
}

@compute @workgroup_size(1)
fn compute_main() {
  any_e755c1();
}
