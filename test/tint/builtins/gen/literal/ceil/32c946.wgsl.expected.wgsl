fn ceil_32c946() {
  var res = ceil(vec3(1.5));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ceil_32c946();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ceil_32c946();
}

@compute @workgroup_size(1)
fn compute_main() {
  ceil_32c946();
}
