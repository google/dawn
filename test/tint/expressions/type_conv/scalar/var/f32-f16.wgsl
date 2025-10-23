// flags:  --hlsl-shader-model 62
enable f16;
var<private> u = f32(1.0f);

@compute @workgroup_size(1)
fn f() {
    let v : f16 = f16(u);
}
