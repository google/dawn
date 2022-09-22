@group(1) @binding(0) var arg_0 : texture_2d<f32>;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSampleBaseClampToEdge_9ca02c() {
  var arg_2 = vec2<f32>();
  var res : vec4<f32> = textureSampleBaseClampToEdge(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureSampleBaseClampToEdge_9ca02c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureSampleBaseClampToEdge_9ca02c();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureSampleBaseClampToEdge_9ca02c();
}
