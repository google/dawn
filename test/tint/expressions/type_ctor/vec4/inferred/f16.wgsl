// flags:  --hlsl-shader-model 62
enable f16;
var<private> v = vec4(0.0h, 1.0h, 2.0h, 3.0h);

@compute @workgroup_size(1)
fn main() {
    _ = v;
}
