enable f16;

fn transpose_844869() {
  var res : mat4x4<f16> = transpose(mat4x4<f16>(f16(), f16(), f16(), f16(), f16(), f16(), f16(), f16(), f16(), f16(), f16(), f16(), f16(), f16(), f16(), f16()));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  transpose_844869();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  transpose_844869();
}

@compute @workgroup_size(1)
fn compute_main() {
  transpose_844869();
}
