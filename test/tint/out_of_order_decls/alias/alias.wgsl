type T1 = T2;
type T2 = i32;

@stage(fragment)
fn f() {
  var v : T1;
}
