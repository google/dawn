alias a = vec3f;

@compute @workgroup_size(1)
fn f() {
  {
    let vec3f = 1;
    let b = vec3f;
  }
  let c = a();
  let d = vec3f();
}
