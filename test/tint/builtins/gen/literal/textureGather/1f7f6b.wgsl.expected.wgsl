@group(1) @binding(0) var arg_0 : texture_depth_2d;

@group(1) @binding(1) var arg_1 : sampler;

fn textureGather_1f7f6b() {
  var res : vec4<f32> = textureGather(arg_0, arg_1, vec2<f32>(1.0f), vec2<i32>(1i));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureGather_1f7f6b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureGather_1f7f6b();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureGather_1f7f6b();
}
