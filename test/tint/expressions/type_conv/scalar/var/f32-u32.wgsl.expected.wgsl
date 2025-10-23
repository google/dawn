var<private> u = f32(1.0f);

@compute @workgroup_size(1)
fn f() {
  let v : u32 = u32(u);
}
