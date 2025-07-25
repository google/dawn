// flags:  --hlsl-shader-model 62
enable f16;

@compute @workgroup_size(1)
fn f() {
    const a : vec4<f16> = vec4<f16>(1.0h, 2.0h, 3.0h, -4.0h);
    let b : vec2<f32> = bitcast<vec2<f32>>(a);
}
