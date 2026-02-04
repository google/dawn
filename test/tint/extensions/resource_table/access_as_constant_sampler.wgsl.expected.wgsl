enable chromium_experimental_resource_table;

@fragment
fn fs() -> @location(0) vec4f {
  let t = getResource<texture_2d<f32>>(0);
  let s = getResource<sampler>(1);
  return textureSample(t, s, vec2f(0));
}
