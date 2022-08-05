enable f16;

fn transpose_5f36bf() {
  var arg_0 = mat4x3<f16>(f16(), f16(), f16(), f16(), f16(), f16(), f16(), f16(), f16(), f16(), f16(), f16());
  var res : mat3x4<f16> = transpose(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  transpose_5f36bf();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  transpose_5f36bf();
}

@compute @workgroup_size(1)
fn compute_main() {
  transpose_5f36bf();
}
