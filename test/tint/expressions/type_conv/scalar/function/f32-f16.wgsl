// flags:  --hlsl-shader-model 62
enable f16;
var<private> t : f32;
fn m() -> f32 {
    t = 1.0f;
    return f32(t);
}

@compute @workgroup_size(1)
fn f() {
    var v : f16 = f16(m());
}
