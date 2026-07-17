struct Immediate {
  color : vec3<f32>,
  count : u32,
}

var<immediate> data : Immediate;

@group(0) @binding(0) var<storage, read_write> output : vec4<f32>;

@compute @workgroup_size(1)
fn main() {
  output = vec4<f32>(data.color, f32(data.count));
}
