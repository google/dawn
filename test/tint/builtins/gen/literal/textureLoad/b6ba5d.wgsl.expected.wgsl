@group(1) @binding(0) var arg_0 : texture_depth_2d_array;

fn textureLoad_b6ba5d() {
  var res : f32 = textureLoad(arg_0, vec2<u32>(), 1i, 1i);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_b6ba5d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_b6ba5d();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_b6ba5d();
}
