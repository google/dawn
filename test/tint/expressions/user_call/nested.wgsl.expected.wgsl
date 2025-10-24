@compute @workgroup_size(1)
fn a() {
  b();
}

fn b() {
  c();
}

fn c() {
  d();
}

fn d() {
}
