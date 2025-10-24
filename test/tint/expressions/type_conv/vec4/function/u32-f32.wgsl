var<private> t : u32;
fn m() -> vec4<u32> {
    t = 1u;
    return vec4<u32>(t);
}

@compute @workgroup_size(1)
fn f() {
    var v : vec4<f32> = vec4<f32>(m());
}
