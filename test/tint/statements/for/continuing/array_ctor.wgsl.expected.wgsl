@compute @workgroup_size(1)
fn f() {
  var i : i32;
  for(; false; i = (i + array<i32, 1>(1)[0])) {
  }
}
