// flags:  --hlsl-shader-model 62
enable f16;

@compute @workgroup_size(1)
fn f() {
    let a : vec2<f32> = vec2<f32>(2.003662109375f, -513.03125f);
    let b : vec4<f16> = bitcast<vec4<f16>>(a);
}
