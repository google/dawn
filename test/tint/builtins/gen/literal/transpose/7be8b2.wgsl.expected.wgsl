enable f16;

fn transpose_7be8b2() {
  var res : mat2x2<f16> = transpose(mat2x2<f16>(1.0h, 1.0h, 1.0h, 1.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : mat2x2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  transpose_7be8b2();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  transpose_7be8b2();
}

@compute @workgroup_size(1)
fn compute_main() {
  transpose_7be8b2();
}
