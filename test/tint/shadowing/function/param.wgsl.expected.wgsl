fn a(a : i32) {
  let b = a;
}

@compute @workgroup_size(1)
fn main() {
  a(1);
}
