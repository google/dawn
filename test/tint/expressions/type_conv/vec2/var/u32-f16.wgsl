// flags:  --hlsl-shader-model 62
enable f16;
var<private> u = vec2<u32>(1u);

@compute @workgroup_size(1)
fn f() {
    let v : vec2<f16> = vec2<f16>(u);
}
