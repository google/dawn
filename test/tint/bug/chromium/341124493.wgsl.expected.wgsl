@compute @workgroup_size(1)
fn F() {
  var b : bool;
  if ((false && select(!(b), true, true))) {
  }
}
