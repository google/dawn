@group(1) @binding(0) var arg_0 : texture_depth_2d_array;

@group(1) @binding(1) var arg_1 : sampler;

fn textureGather_d90605() {
  var arg_2 = vec2<f32>();
  var arg_3 = 1;
  const arg_4 = vec2<i32>();
  var res : vec4<f32> = textureGather(arg_0, arg_1, arg_2, arg_3, arg_4);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureGather_d90605();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureGather_d90605();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureGather_d90605();
}
