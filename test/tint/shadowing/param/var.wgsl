fn f(a : i32) {
  {
    var a = a;
    var b = a;
  }
}

@compute @workgroup_size(1)
fn main() {
    f(1);
}
