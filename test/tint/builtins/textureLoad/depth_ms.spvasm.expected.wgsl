@group(1) @binding(0) var arg_0 : texture_depth_multisampled_2d;

var<private> tint_symbol_1 : vec4<f32> = vec4<f32>();

fn textureLoad_6273b1() {
  var res : f32 = 0.0;
  let x_17 : vec4<f32> = vec4<f32>(textureLoad(arg_0, vec2<i32>(), 1i), 0.0, 0.0, 0.0);
  res = x_17.x;
  return;
}

fn tint_symbol_2(tint_symbol : vec4<f32>) {
  tint_symbol_1 = tint_symbol;
  return;
}

fn vertex_main_1() {
  textureLoad_6273b1();
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
  textureLoad_6273b1();
  return;
}

@stage(fragment)
fn fragment_main() {
  fragment_main_1();
}

fn compute_main_1() {
  textureLoad_6273b1();
  return;
}

@stage(compute) @workgroup_size(1i, 1i, 1i)
fn compute_main() {
  compute_main_1();
}
