@group(1) @binding(0) var arg_0 : texture_depth_multisampled_2d;

var<private> tint_symbol_1 : vec4<f32> = vec4<f32>();

fn textureNumSamples_a3c8a0() {
  var res : i32 = 0i;
  let x_16 : i32 = textureNumSamples(arg_0);
  res = x_16;
  return;
}

fn tint_symbol_2(tint_symbol : vec4<f32>) {
  tint_symbol_1 = tint_symbol;
  return;
}

fn vertex_main_1() {
  textureNumSamples_a3c8a0();
  tint_symbol_2(vec4<f32>());
  return;
}

struct vertex_main_out {
  @builtin(position)
  tint_symbol_1_1 : vec4<f32>,
}

@stage(vertex)
fn vertex_main() -> vertex_main_out {
  vertex_main_1();
  return vertex_main_out(tint_symbol_1);
}

fn fragment_main_1() {
  textureNumSamples_a3c8a0();
  return;
}

@stage(fragment)
fn fragment_main() {
  fragment_main_1();
}

fn compute_main_1() {
  textureNumSamples_a3c8a0();
  return;
}

@stage(compute) @workgroup_size(1, 1, 1)
fn compute_main() {
  compute_main_1();
}
