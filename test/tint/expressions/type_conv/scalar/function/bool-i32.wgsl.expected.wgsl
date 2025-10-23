var<private> t : bool;

fn m() -> bool {
  t = true;
  return bool(t);
}

@compute @workgroup_size(1)
fn f() {
  var v : i32 = i32(m());
}
