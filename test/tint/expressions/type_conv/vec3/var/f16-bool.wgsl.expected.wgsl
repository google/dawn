enable f16;

var<private> u = vec3<f16>(1.0h);

@compute @workgroup_size(1)
fn f() {
  let v : vec3<bool> = vec3<bool>(u);
}
