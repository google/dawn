fn select_089657() {
  var res = select(vec3(1.0), vec3(1.0), true);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_089657();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_089657();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_089657();
}
