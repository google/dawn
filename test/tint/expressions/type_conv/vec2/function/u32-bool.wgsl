var<private> t : u32;
fn m() -> vec2<u32> {
    t = 1u;
    return vec2<u32>(t);
}

@compute @workgroup_size(1)
fn f() {
    var v : vec2<bool> = vec2<bool>(m());
}
