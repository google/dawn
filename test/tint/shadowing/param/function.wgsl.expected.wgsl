fn a(a : i32) {
  {
    var a = a;
    var b = a;
  }
}

@compute @workgroup_size(1)
fn main() {
  a(1);
}
