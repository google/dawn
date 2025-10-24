enable f16;

@compute @workgroup_size(1)
fn f() {
  var v = mat2x2<f16>();
}
