@compute @workgroup_size(1)
fn f() {
  var i : i32;
  for (; false; i = i + 1) {}
}
