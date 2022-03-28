type T = S;

struct S {
  m : i32,
}

@stage(fragment)
fn f() {
  var v : T;
}
