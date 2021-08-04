var<private> v : f32;

fn x(p : ptr<private, f32>) {
  (*p) = 0.0;
}

fn g() {
  x(&v);
}

[[stage(fragment)]]
fn f() {
  g();
}
