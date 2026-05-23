enable chromium_experimental_resource_table;

@group(0) @binding(0) var t : texture_2d<f32>;

@fragment
fn fs() -> @location(0) vec4f {
  let s = getResource<sampler>(0);
  return textureSample(t, s, vec2f(0));
}


