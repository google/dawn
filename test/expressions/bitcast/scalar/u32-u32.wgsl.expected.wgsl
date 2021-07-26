[[stage(compute), workgroup_size(1)]]
fn f() {
  let a : u32 = 1u;
  let b : u32 = bitcast<u32>(a);
}
