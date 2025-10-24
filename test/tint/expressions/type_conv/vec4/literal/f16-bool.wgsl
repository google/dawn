// flags:  --hlsl-shader-model 62
enable f16;
var<private> u : vec4<bool> = vec4<bool>(vec4<f16>(1.0h));

@compute @workgroup_size(1)
fn main() {
    _ = u;
}
