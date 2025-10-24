@compute @workgroup_size(1)
fn f() {
  for(var i : i32 = array<i32, 1>(1)[0]; false; ) {
  }
}
