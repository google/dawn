fn f(vec3f : vec3f) {
  let b = vec3f;
}

@compute @workgroup_size(1)
fn main() {
  f(vec3f());
}
