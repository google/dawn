@group(1) @binding(1) var arg_1 : texture_2d_array<i32>;

@group(1) @binding(2) var arg_2 : sampler;

fn textureGather_4e8ac5() {
  var res : vec4<i32> = textureGather(1i, arg_1, arg_2, vec2<f32>(1.0f), 1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureGather_4e8ac5();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureGather_4e8ac5();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureGather_4e8ac5();
}
