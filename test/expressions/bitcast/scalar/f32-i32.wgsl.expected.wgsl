[[stage(compute), workgroup_size(1)]]
fn f() {
  let a : f32 = 1.0;
  let b : i32 = bitcast<i32>(a);
}
