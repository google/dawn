// flags:  --hlsl-shader-model 62
enable f16;
var<private> u = vec2<f32>(1.0f);
fn f() {
    let v : vec2<f16> = vec2<f16>(u);
}