var<private> u = bool(true);

@compute @workgroup_size(1)
fn f() {
  let v : i32 = i32(u);
}
