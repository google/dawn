enable f16;

fn transpose_06794e() {
  var arg_0 = mat3x3<f16>(f16(), f16(), f16(), f16(), f16(), f16(), f16(), f16(), f16());
  var res : mat3x3<f16> = transpose(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  transpose_06794e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  transpose_06794e();
}

@compute @workgroup_size(1)
fn compute_main() {
  transpose_06794e();
}
