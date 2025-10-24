struct S {
  i : i32,
}

@compute @workgroup_size(1)
fn f() {
  var i : i32;
  for(; (i < S(1).i); ) {
  }
}
