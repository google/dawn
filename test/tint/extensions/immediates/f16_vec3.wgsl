// flags:  --hlsl-shader-model 6.2
enable f16;

var<immediate> value: vec3<f16>;

@group(0) @binding(0)
var<storage, read_write> output: vec3<f32>;

@compute @workgroup_size(1)
fn main() {
  output = vec3<f32>(value);
}
