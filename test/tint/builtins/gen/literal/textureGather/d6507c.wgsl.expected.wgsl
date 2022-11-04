@group(1) @binding(1) var arg_1 : texture_2d<f32>;

@group(1) @binding(2) var arg_2 : sampler;

fn textureGather_d6507c() {
  var res : vec4<f32> = textureGather(1u, arg_1, arg_2, vec2<f32>(1.0f), vec2<i32>(1i));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureGather_d6507c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureGather_d6507c();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureGather_d6507c();
}
