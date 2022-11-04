@group(1) @binding(0) var arg_0 : texture_2d_array<f32>;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSampleBias_87915c() {
  var res : vec4<f32> = textureSampleBias(arg_0, arg_1, vec2<f32>(1.0f), 1u, 1.0f, vec2<i32>(1i));
}

@fragment
fn fragment_main() {
  textureSampleBias_87915c();
}
