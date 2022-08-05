enable f16;

fn transpose_d6faec() {
  var res : mat3x2<f16> = transpose(mat2x3<f16>(f16(), f16(), f16(), f16(), f16(), f16()));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  transpose_d6faec();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  transpose_d6faec();
}

@compute @workgroup_size(1)
fn compute_main() {
  transpose_d6faec();
}
