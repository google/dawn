// flags:  --hlsl-shader-model 62
enable f16;
var<private> u : f16 = f16(f32(1.0f));

@compute @workgroup_size(1)
fn main() {
    _ = u;
}
