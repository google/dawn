@group(1) @binding(0) var arg_0 : texture_depth_2d_array;

@group(1) @binding(1) var arg_1 : sampler_comparison;

fn textureGatherCompare_144a9a() {
  var res : vec4<f32> = textureGatherCompare(arg_0, arg_1, vec2<f32>(1.0f), 1u, 1.0f, vec2<i32>(1i));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureGatherCompare_144a9a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureGatherCompare_144a9a();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureGatherCompare_144a9a();
}
