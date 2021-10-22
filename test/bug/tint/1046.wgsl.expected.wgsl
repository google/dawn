struct PointLight {
  position : vec4<f32>;
};

[[block]]
struct PointLights {
  values : [[stride(16)]] array<PointLight>;
};

[[block]]
struct Uniforms {
  worldView : mat4x4<f32>;
  proj : mat4x4<f32>;
  numPointLights : u32;
  color_source : u32;
  color : vec4<f32>;
};

[[binding(0), group(0)]] var<uniform> uniforms : Uniforms;

[[binding(1), group(0)]] var<storage, read> pointLights : PointLights;

[[binding(2), group(0)]] var mySampler : sampler;

[[binding(3), group(0)]] var myTexture : texture_2d<f32>;

struct FragmentInput {
  [[builtin(position)]]
  position : vec4<f32>;
  [[location(0)]]
  view_position : vec4<f32>;
  [[location(1)]]
  normal : vec4<f32>;
  [[location(2)]]
  uv : vec2<f32>;
  [[location(3)]]
  color : vec4<f32>;
};

struct FragmentOutput {
  [[location(0)]]
  color : vec4<f32>;
};

fn getColor(fragment : FragmentInput) -> vec4<f32> {
  var color : vec4<f32>;
  if ((uniforms.color_source == 0u)) {
    color = fragment.color;
  } elseif ((uniforms.color_source == 1u)) {
    color = fragment.normal;
    color.a = 1.0;
  } elseif ((uniforms.color_source == 2u)) {
    color = uniforms.color;
  } elseif ((uniforms.color_source == 3u)) {
    color = textureSample(myTexture, mySampler, fragment.uv);
  }
  return color;
}

[[stage(fragment)]]
fn main(fragment : FragmentInput) -> FragmentOutput {
  var output : FragmentOutput;
  output.color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  _ = uniforms;
  _ = mySampler;
  _ = myTexture;
  _ = &(pointLights);
  return output;
}
