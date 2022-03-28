struct S1 {
  m : S2,
}

struct S2 {
  m : i32,
}

@stage(fragment)
fn f() {
  var v : S1;
}
