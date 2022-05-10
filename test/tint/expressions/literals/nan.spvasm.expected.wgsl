var<private> out_var_SV_TARGET : vec4<f32>;

fn main_1() {
  out_var_SV_TARGET = vec4<f32>(0x1.9p+128, 0x1.9p+128, 0x1.9p+128, 0x1.9p+128);
  return;
}

struct main_out {
  @location(0)
  out_var_SV_TARGET_1 : vec4<f32>,
}

@stage(fragment)
fn main() -> main_out {
  main_1();
  return main_out(out_var_SV_TARGET);
}
