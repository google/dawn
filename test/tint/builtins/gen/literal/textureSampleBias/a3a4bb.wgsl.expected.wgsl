@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@group(1) @binding(0) var arg_0 : texture_cube_array<f32, unfilterable>;

@group(1) @binding(1) var arg_1 : sampler<non_filtering>;

fn textureSampleBias_a3a4bb() -> vec4<f32> {
  var res : vec4<f32> = textureSampleBias(arg_0, arg_1, vec3<f32>(1.0f), 1i, 1.0f);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureSampleBias_a3a4bb();
}
