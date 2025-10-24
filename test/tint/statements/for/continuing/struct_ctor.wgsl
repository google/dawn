struct S {
  i : i32,
};

@compute @workgroup_size(1)
fn f() {
  for (var i = 0; false; i = i + S(1).i) {}
}
