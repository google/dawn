fn select_b93806() {
  var res = select(vec3(1), vec3(1), vec3<bool>(true));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_b93806();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_b93806();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_b93806();
}
