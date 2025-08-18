var<private> tint_pointsize : f32 = 1.0f;

@group(1u) @binding(0u) var arg_0 : texture_depth_multisampled_2d;

var<private> tint_symbol_1 : vec4<f32> = vec4<f32>();

fn textureDimensions_f60bdb() {
  var res : vec2<i32> = vec2<i32>();
  res = vec2<i32>(textureDimensions(arg_0));
}

fn tint_symbol_2(tint_symbol : vec4<f32>) {
  tint_symbol_1 = tint_symbol;
}

fn vertex_main_inner() {
  tint_pointsize = 1.0f;
  textureDimensions_f60bdb();
  tint_symbol_2(vec4<f32>());
}

@fragment
fn fragment_main() {
  textureDimensions_f60bdb();
}

@compute @workgroup_size(1u, 1u, 1u)
fn compute_main() {
  textureDimensions_f60bdb();
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  vertex_main_inner();
  return tint_symbol_1;
}
