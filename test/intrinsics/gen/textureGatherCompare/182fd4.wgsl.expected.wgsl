[[group(1), binding(0)]] var arg_0 : texture_depth_cube;

[[group(1), binding(1)]] var arg_1 : sampler_comparison;

fn textureGatherCompare_182fd4() {
  var res : vec4<f32> = textureGatherCompare(arg_0, arg_1, vec3<f32>(), 1.0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureGatherCompare_182fd4();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureGatherCompare_182fd4();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureGatherCompare_182fd4();
}
