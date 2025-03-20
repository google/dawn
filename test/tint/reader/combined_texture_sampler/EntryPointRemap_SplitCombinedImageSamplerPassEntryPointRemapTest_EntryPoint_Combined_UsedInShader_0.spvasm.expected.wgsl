var<private> in_var : vec4f;

var<private> out_var : vec4f;

@group(0) @binding(1) var combined_var_sampler : sampler;

@group(0) @binding(0) var combined_var_image : texture_2d<f32>;

fn main_1() {
  return;
}

struct main_out {
  @location(0)
  out_var_1 : vec4f,
}

@fragment
fn main(@builtin(position) in_var_param : vec4f) -> main_out {
  in_var = in_var_param;
  main_1();
  return main_out(out_var);
}
