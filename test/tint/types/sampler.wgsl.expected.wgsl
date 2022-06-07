@group(0) @binding(0) var s : sampler;

@group(0) @binding(1) var sc : sampler_comparison;

@compute @workgroup_size(1)
fn main() {
  _ = s;
  _ = sc;
}
