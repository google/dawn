[[group(1), binding(0)]] var arg_0 : texture_depth_2d_array;

[[group(1), binding(1)]] var arg_1 : sampler_comparison;

fn textureSampleCompareLevel_011a8f() {
  var res : f32 = textureSampleCompareLevel(arg_0, arg_1, vec2<f32>(), 1, 1.0, vec2<i32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureSampleCompareLevel_011a8f();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureSampleCompareLevel_011a8f();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureSampleCompareLevel_011a8f();
}
