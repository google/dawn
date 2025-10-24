alias a = vec3f;

@compute @workgroup_size(1)
fn f() {
  {
    const vec3f = 1;
    const b = vec3f;
  }
  const c = a();
  const d = vec3f();
}
