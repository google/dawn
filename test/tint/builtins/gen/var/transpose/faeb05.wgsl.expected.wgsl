enable f16;

fn transpose_faeb05() {
  var arg_0 = mat2x4<f16>(f16(), f16(), f16(), f16(), f16(), f16(), f16(), f16());
  var res : mat4x2<f16> = transpose(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  transpose_faeb05();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  transpose_faeb05();
}

@compute @workgroup_size(1)
fn compute_main() {
  transpose_faeb05();
}
