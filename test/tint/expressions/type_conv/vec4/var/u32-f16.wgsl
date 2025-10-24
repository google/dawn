// flags:  --hlsl-shader-model 62
enable f16;
var<private> u = vec4<u32>(1u);

@compute @workgroup_size(1)
fn f() {
    let v : vec4<f16> = vec4<f16>(u);
}
