// flags:  --hlsl-shader-model 62
enable f16;

@compute @workgroup_size(1)
fn f() {
    const a : vec2<i32> = vec2<i32>(1073757184i, -1006616064i);
    let b : vec4<f16> = bitcast<vec4<f16>>(a);
}
