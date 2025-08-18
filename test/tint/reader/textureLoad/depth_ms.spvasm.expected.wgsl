var<private> tint_pointsize : f32 = 1.0f;

@group(1u) @binding(0u) var arg_0 : texture_depth_multisampled_2d;

var<private> tint_symbol_1 : vec4<f32> = vec4<f32>();

fn textureLoad_6273b1() {
  var res : f32 = 0.0f;
  res = vec4<f32>(textureLoad(arg_0, vec2<i32>(), 1i), 0.0f, 0.0f, 0.0f).x;
}

fn tint_symbol_2(tint_symbol : vec4<f32>) {
  tint_symbol_1 = tint_symbol;
}

fn vertex_main_inner() {
  tint_pointsize = 1.0f;
  textureLoad_6273b1();
  tint_symbol_2(vec4<f32>());
}

@fragment
fn fragment_main() {
  textureLoad_6273b1();
}

@compute @workgroup_size(1u, 1u, 1u)
fn compute_main() {
  textureLoad_6273b1();
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  vertex_main_inner();
  return tint_symbol_1;
}
