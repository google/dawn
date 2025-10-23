// flags:  --hlsl-shader-model 62
enable f16;
var<private> t : u32;
fn m() -> u32 {
    t = 1u;
    return u32(t);
}

@compute @workgroup_size(1)
fn f() {
    var v : f16 = f16(m());
}
