@group(1) @binding(0) var arg_0 : texture_2d_array<f32>;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSample_17e988() {
  var arg_2 = vec2<f32>();
  var arg_3 = 1i;
  const arg_4 = vec2<i32>();
  var res : vec4<f32> = textureSample(arg_0, arg_1, arg_2, arg_3, arg_4);
}

@fragment
fn fragment_main() {
  textureSample_17e988();
}
