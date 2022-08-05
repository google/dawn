enable f16;

fn determinant_d7c86f() {
  var arg_0 = mat3x3<f16>(f16(), f16(), f16(), f16(), f16(), f16(), f16(), f16(), f16());
  var res : f16 = determinant(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  determinant_d7c86f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  determinant_d7c86f();
}

@compute @workgroup_size(1)
fn compute_main() {
  determinant_d7c86f();
}
