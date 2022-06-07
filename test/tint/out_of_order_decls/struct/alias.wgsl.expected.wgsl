struct S {
  m : T,
}

type T = i32;

@fragment
fn f() {
  var v : S;
}
