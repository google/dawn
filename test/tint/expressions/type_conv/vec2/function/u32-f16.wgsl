// flags:  --hlsl-shader-model 62
enable f16;
var<private> t : u32;
fn m() -> vec2<u32> {
    t = 1u;
    return vec2<u32>(t);
}
fn f() {
    var v : vec2<f16> = vec2<f16>(m());
}