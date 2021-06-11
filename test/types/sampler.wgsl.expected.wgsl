[[group(0), binding(0)]] var s : sampler;

[[group(0), binding(1)]] var sc : sampler_comparison;

[[stage(compute)]]
fn main() {
  ignore(s);
  ignore(sc);
}
