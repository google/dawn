@group(1) @binding(1) var arg_1 : texture_2d<i32>;

@group(1) @binding(2) var arg_2 : sampler;

fn textureGather_7c3828() {
  var res : vec4<i32> = textureGather(1i, arg_1, arg_2, vec2<f32>(1.0f), vec2<i32>(1i));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureGather_7c3828();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureGather_7c3828();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureGather_7c3828();
}
