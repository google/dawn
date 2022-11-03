fn floor_953774() {
  const arg_0 = vec3(1.5);
  var res = floor(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  floor_953774();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  floor_953774();
}

@compute @workgroup_size(1)
fn compute_main() {
  floor_953774();
}
