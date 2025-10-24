var<private> t : i32;
fn m() -> vec3<i32> {
    t = 1i;
    return vec3<i32>(t);
}

@compute @workgroup_size(1)
fn f() {
    var v : vec3<bool> = vec3<bool>(m());
}
