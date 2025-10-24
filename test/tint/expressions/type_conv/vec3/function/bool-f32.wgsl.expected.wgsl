var<private> t : bool;

fn m() -> vec3<bool> {
  t = true;
  return vec3<bool>(t);
}

@compute @workgroup_size(1)
fn f() {
  var v : vec3<f32> = vec3<f32>(m());
}
