@compute @workgroup_size(1)
fn f() {
  _ = frexp(vec2(2.0));
}
