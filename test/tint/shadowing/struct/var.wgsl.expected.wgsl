struct a {
  a : i32,
}

@compute @workgroup_size(1)
fn f() {
  {
    var a : a = a();
    var b = a;
  }
  var a : a = a();
  var b = a;
}
