enable f16;

fn transpose_b9ad1f() {
  var arg_0 = mat3x2<f16>(1.0h, 1.0h, 1.0h, 1.0h, 1.0h, 1.0h);
  var res : mat2x3<f16> = transpose(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : mat2x3<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  transpose_b9ad1f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  transpose_b9ad1f();
}

@compute @workgroup_size(1)
fn compute_main() {
  transpose_b9ad1f();
}
