fn f(x_100 : u32, x_202 : texture_2d<f32>, x_203 : sampler, x_110 : f32) -> vec4f {
  let x_200 = textureSampleLevel(x_202, x_203, vec2f(), 0.0f);
  return x_200;
}

fn main_1() {
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
