// flags:  --hlsl-shader-model 62
enable f16;
var<private> u = vec4<f32>(1.0f);
fn f() {
    let v : vec4<f16> = vec4<f16>(u);
}