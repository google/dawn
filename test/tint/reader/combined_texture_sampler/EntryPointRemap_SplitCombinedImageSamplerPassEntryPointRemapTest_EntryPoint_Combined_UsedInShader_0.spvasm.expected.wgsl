var<private> out_var : vec4<f32>;

@group(0u) @binding(1u) var combined_var_sampler : sampler;

@group(0u) @binding(0u) var combined_var_image : texture_2d<f32>;

fn main_inner(in_var : vec4<f32>) {
  _ = combined_var_image;
  _ = combined_var_sampler;
}

@fragment
fn main(@builtin(position) in_var : vec4<f32>) -> @location(0u) vec4<f32> {
  main_inner(in_var);
  return out_var;
}
