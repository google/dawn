fn a(x : ptr<function, i32>) {
}

fn b(x : ptr<function, i32>) {
  a(x);
}

@compute @workgroup_size(1)
fn main() {
  var c = 1;
  b(&(c));
}
