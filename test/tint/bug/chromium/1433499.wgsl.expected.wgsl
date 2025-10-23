fn f(p : ptr<function, f32>) {
  let x = p;
}

@compute @workgroup_size(1)
fn main() {
  var a = 1.0f;
  f(&(a));
}
