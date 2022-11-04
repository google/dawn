fn select_2c96d4() {
  var res = select(vec3(1.0), vec3(1.0), vec3<bool>(true));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_2c96d4();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_2c96d4();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_2c96d4();
}
