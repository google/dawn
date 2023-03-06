enable f16;

fn transpose_844869() {
  var arg_0 = mat4x4<f16>(1.0h, 1.0h, 1.0h, 1.0h, 1.0h, 1.0h, 1.0h, 1.0h, 1.0h, 1.0h, 1.0h, 1.0h, 1.0h, 1.0h, 1.0h, 1.0h);
  var res : mat4x4<f16> = transpose(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : mat4x4<f16>;

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
