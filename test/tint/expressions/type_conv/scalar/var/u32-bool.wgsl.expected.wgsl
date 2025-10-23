var<private> u = u32(1u);

@compute @workgroup_size(1)
fn f() {
  let v : bool = bool(u);
}
