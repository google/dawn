fn length_602a17() {
  var res : f32 = length(0.0f);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  length_602a17();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  length_602a17();
}

@compute @workgroup_size(1)
fn compute_main() {
  length_602a17();
}
