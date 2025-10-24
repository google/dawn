enable f16;

@compute @workgroup_size(1)
fn f() {
  var v = mat3x4<f16>();
}
