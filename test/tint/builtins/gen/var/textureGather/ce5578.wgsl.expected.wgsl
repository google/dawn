@group(1) @binding(1) var arg_1 : texture_2d_array<u32>;

@group(1) @binding(2) var arg_2 : sampler;

fn textureGather_ce5578() {
  const arg_0 = 1i;
  var arg_3 = vec2<f32>(1.0f);
  var arg_4 = 1u;
  const arg_5 = vec2<i32>(1i);
  var res : vec4<u32> = textureGather(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureGather_ce5578();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureGather_ce5578();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureGather_ce5578();
}
