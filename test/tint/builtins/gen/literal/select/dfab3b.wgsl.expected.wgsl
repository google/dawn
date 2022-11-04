fn select_dfab3b() {
  var res = select(vec3(1), vec3(1), true);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_dfab3b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_dfab3b();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_dfab3b();
}
