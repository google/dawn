fn length_c2c544() {
  var res = length(vec4(0.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  length_c2c544();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  length_c2c544();
}

@compute @workgroup_size(1)
fn compute_main() {
  length_c2c544();
}
