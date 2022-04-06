struct S {
  m : T,
}

type T = i32;

@stage(fragment)
fn f() {
  var v : S;
}
