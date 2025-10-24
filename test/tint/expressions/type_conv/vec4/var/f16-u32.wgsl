// flags:  --hlsl-shader-model 62
enable f16;
var<private> u = vec4<f16>(1.0h);

@compute @workgroup_size(1)
fn f() {
    let v : vec4<u32> = vec4<u32>(u);
}
