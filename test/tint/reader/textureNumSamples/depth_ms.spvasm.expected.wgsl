var<private> tint_pointsize : f32 = 1.0f;

@group(1u) @binding(0u) var arg_0 : texture_depth_multisampled_2d;

var<private> tint_symbol_1 : vec4<f32> = vec4<f32>();

fn textureNumSamples_a3c8a0() {
  var res : i32 = 0i;
  res = i32(textureNumSamples(arg_0));
}

fn tint_symbol_2(tint_symbol : vec4<f32>) {
  tint_symbol_1 = tint_symbol;
}

fn vertex_main_inner() {
  tint_pointsize = 1.0f;
  textureNumSamples_a3c8a0();
  tint_symbol_2(vec4<f32>());
}

@fragment
fn fragment_main() {
  textureNumSamples_a3c8a0();
}

@compute @workgroup_size(1u, 1u, 1u)
fn compute_main() {
  textureNumSamples_a3c8a0();
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  vertex_main_inner();
  return tint_symbol_1;
}
