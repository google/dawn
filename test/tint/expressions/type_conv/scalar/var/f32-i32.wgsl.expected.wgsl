var<private> u = f32(1.0f);

@compute @workgroup_size(1)
fn f() {
  let v : i32 = i32(u);
}
