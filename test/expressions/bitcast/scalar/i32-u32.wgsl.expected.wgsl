[[stage(compute), workgroup_size(1)]]
fn f() {
  let a : i32 = 1;
  let b : u32 = bitcast<u32>(a);
}
