struct a {
  b : i32,
};
struct a__ {
  b__ : i32,
};
fn f() {
  let c = a__();
  let d = c.b__;
}
