@group(1) @binding(0) var arg_0 : texture_depth_multisampled_2d;

fn textureLoad_4db25c() {
  var res : f32 = textureLoad(arg_0, vec2<u32>(), 1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_4db25c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_4db25c();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_4db25c();
}
