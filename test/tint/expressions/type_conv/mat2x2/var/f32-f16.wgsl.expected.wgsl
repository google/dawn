enable f16;

var<private> u = mat2x2<f32>(1.0f, 2.0f, 3.0f, 4.0f);

@compute @workgroup_size(1)
fn f() {
  var v : mat2x2<f16> = mat2x2<f16>(u);
}
