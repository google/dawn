// flags:  --hlsl-shader-model 62
enable f16;

@compute @workgroup_size(1)
fn f() {
    let a : f32 = f32(2.003662109375f);
    let b : vec2<f16> = bitcast<vec2<f16>>(a);
}
