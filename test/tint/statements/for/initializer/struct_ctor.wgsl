struct S {
  i : i32,
};

@compute @workgroup_size(1)
fn f() {
  for (var i : i32 = S(1).i; false;) {}
}
