@compute @workgroup_size(1)
fn a() {
  {
    var a = 1;
    var b = a;
  }
  let a = 1;
  let b = a;
}
