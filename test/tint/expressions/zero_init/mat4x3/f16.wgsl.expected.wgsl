enable f16;

@compute @workgroup_size(1)
fn f() {
  var v = mat4x3<f16>();
}
