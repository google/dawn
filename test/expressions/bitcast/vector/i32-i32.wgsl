[[stage(compute), workgroup_size(1)]]
fn f() {
    let a : vec3<i32> = vec3<i32>(1, 2, 3);
    let b : vec3<i32> = bitcast<vec3<i32>>(a);
}
