fn c() {
  var a = 1;
  a = (a + 2);
}

@compute @workgroup_size(1)
fn b() {
  c();
  c();
}
