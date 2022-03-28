struct a {
  b : i32,
};
struct _a {
  _b : i32,
};
fn f() {
  let c = _a();
  let d = c._b;
}
