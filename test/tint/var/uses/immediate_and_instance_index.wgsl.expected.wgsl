var<immediate> a : f32;

@vertex
fn main(@builtin(instance_index) b : u32) -> @builtin(position) vec4<f32> {
  return vec4<f32>((a + f32(b)));
}
