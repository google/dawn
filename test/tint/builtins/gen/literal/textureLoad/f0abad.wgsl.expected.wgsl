@group(1) @binding(0) var arg_0 : texture_multisampled_2d<f32>;

fn textureLoad_f0abad() {
  var res : vec4<f32> = textureLoad(arg_0, vec2<u32>(), 1i);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_f0abad();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_f0abad();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_f0abad();
}
