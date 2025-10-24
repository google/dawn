var<private> u = vec3<i32>(1i);

@compute @workgroup_size(1)
fn f() {
    let v : vec3<bool> = vec3<bool>(u);
}
