var<private> t : bool;

fn m() -> bool {
  t = true;
  return bool(t);
}

@compute @workgroup_size(1)
fn f() {
  var v : u32 = u32(m());
}
