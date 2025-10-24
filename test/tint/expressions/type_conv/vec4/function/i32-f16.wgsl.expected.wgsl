enable f16;

var<private> t : i32;

fn m() -> vec4<i32> {
  t = 1i;
  return vec4<i32>(t);
}

@compute @workgroup_size(1)
fn f() {
  var v : vec4<f16> = vec4<f16>(m());
}
