// flags:  --hlsl-shader-model 62
enable f16;

@compute @workgroup_size(1)
fn f() {
    let a : u32 = u32(1073757184u);
    let b : vec2<f16> = bitcast<vec2<f16>>(a);
}
