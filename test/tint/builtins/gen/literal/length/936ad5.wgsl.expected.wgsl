fn length_936ad5() {
  var res = length(0.0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  length_936ad5();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  length_936ad5();
}

@compute @workgroup_size(1)
fn compute_main() {
  length_936ad5();
}
