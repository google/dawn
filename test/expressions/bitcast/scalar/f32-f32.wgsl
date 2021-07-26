[[stage(compute), workgroup_size(1)]]
fn f() {
    let a : f32 = 1.;
    let b : f32 = bitcast<f32>(a);
}
