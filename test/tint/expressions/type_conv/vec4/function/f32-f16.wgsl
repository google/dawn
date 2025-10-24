// flags:  --hlsl-shader-model 62
enable f16;
var<private> t : f32;
fn m() -> vec4<f32> {
    t = 1.0f;
    return vec4<f32>(t);
}

@compute @workgroup_size(1)
fn f() {
    var v : vec4<f16> = vec4<f16>(m());
}
