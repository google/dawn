var<private> u = i32(1i);

@compute @workgroup_size(1)
fn f() {
  let v : u32 = u32(u);
}
