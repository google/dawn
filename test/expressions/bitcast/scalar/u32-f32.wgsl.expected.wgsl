[[stage(compute), workgroup_size(1)]]
fn f() {
  let a : u32 = 1u;
  let b : f32 = bitcast<f32>(a);
}
