// flags:  --hlsl-shader-model 62
enable f16;
var<private> u = f16(1.0h);

@compute @workgroup_size(1)
fn f() {
    let v : f32 = f32(u);
}
