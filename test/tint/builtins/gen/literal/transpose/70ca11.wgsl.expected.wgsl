fn transpose_70ca11() {
  var res = transpose(mat2x3(1.0, 1.0, 1.0, 1.0, 1.0, 1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  transpose_70ca11();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  transpose_70ca11();
}

@compute @workgroup_size(1)
fn compute_main() {
  transpose_70ca11();
}
