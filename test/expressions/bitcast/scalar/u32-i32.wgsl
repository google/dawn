[[stage(compute), workgroup_size(1)]]
fn f() {
    let a : u32 = 1u;
    let b : i32 = bitcast<i32>(a);
}
