[[stage(compute), workgroup_size(1)]]
fn f() {
  let a : i32 = 1;
  let b : i32 = bitcast<i32>(a);
}
