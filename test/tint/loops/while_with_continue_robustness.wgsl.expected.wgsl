fn f() -> i32 {
  var i : i32;
  while((i < 4)) {
    i = (i + 1);
    continue;
  }
  return i;
}

@compute @workgroup_size(1)
fn main() {
  _ = f();
}
