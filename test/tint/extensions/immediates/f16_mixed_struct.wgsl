// flags:  --hlsl-shader-model 6.2
enable f16;

struct Immediate {
  scale: f32,
  offsets: vec2<f16>,
  bias: f16,
}

var<immediate> data: Immediate;

@group(0) @binding(0)
var<storage, read_write> output: vec4<f32>;

@compute @workgroup_size(1)
fn main() {
  output = vec4<f32>(data.scale, vec2<f32>(data.offsets), f32(data.bias));
}
