var<private> t : f32;
fn m() -> vec3<f32> {
    t = 1.0f;
    return vec3<f32>(t);
}

@compute @workgroup_size(1)
fn f() {
    var v : vec3<u32> = vec3<u32>(m());
}
