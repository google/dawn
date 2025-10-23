@compute @workgroup_size(1)
fn f() {
  let v = (false && (array<i32, array(array(6))[0][0]>()[0] == 0));
}
