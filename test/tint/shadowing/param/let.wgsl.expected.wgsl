fn f(a : i32) {
  {
    let a = a;
    let b = a;
  }
}

@compute @workgroup_size(1)
fn main() {
  f(1);
}
