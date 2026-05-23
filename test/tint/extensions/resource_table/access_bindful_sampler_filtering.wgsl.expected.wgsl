enable chromium_experimental_resource_table;

@group(0) @binding(0) var s : sampler;

@fragment
fn fs() -> @location(0) vec4f {
  let t = getResource<texture_2d<f32>>(0);
  return textureSample(t, s, vec2f(0));
}
