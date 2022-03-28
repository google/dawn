struct A {
  B : i32,
};
struct _A {
  _B : i32,
};
fn f() {
  let c = _A();
  let d = c._B;
}
