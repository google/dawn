// flags:  --hlsl-shader-model 62
enable f16;
var<private> u = vec2<f16>(1.0h);
fn f() {
    let v : vec2<u32> = vec2<u32>(u);
}