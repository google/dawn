// flags:  --hlsl-shader-model 62
enable f16;
var<private> u : vec2<f16> = vec2<f16>(vec2<i32>(1i));

@compute @workgroup_size(1)
fn main() {
    _ = u;
}
