enable f16;

var<private> u = vec2<f16>(1.0h);

@compute @workgroup_size(1)
fn f() {
  let v : vec2<f32> = vec2<f32>(u);
}
