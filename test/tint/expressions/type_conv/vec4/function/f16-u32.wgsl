// flags:  --hlsl-shader-model 62
enable f16;
var<private> t : f16;
fn m() -> vec4<f16> {
    t = 1.0h;
    return vec4<f16>(t);
}
fn f() {
    var v : vec4<u32> = vec4<u32>(m());
}