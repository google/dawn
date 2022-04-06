let bloomDir = vec2(0.0, 1.0);

var<private> offsets : array<f32, 3> = array<f32, 3>(0.0, 1.384615421, 3.230769157);

var<private> weights : array<f32, 3> = array<f32, 3>(0.227027029, 0.31621623, 0.07027027);

struct BloomUniforms {
  radius : f32,
  dim : f32,
}

@group(0) @binding(0) var<uniform> bloom : BloomUniforms;

@group(0) @binding(1) var bloomTexture : texture_2d<f32>;

@group(0) @binding(2) var bloomSampler : sampler;

struct FragmentInput {
  @location(0)
  texCoord : vec2<f32>,
}

fn getGaussianBlur(texCoord : vec2<f32>) -> vec4<f32> {
  let texelRadius = (vec2(bloom.radius) / vec2<f32>(textureDimensions(bloomTexture)));
  let step = (bloomDir * texelRadius);
  var sum = vec4(0.0);
  sum = (sum + (textureSample(bloomTexture, bloomSampler, texCoord) * weights[0]));
  sum = (sum + (textureSample(bloomTexture, bloomSampler, (texCoord + (step * 1.0))) * weights[1]));
  sum = (sum + (textureSample(bloomTexture, bloomSampler, (texCoord - (step * 1.0))) * weights[1]));
  sum = (sum + (textureSample(bloomTexture, bloomSampler, (texCoord + (step * 2.0))) * weights[2]));
  sum = (sum + (textureSample(bloomTexture, bloomSampler, (texCoord - (step * 2.0))) * weights[2]));
  return vec4(sum.rgb, 1.0);
}

@group(0) @binding(3) var prevTexture : texture_2d<f32>;

@stage(fragment)
fn fragmentMain(input : FragmentInput) -> @location(0) vec4<f32> {
  let blurColor = getGaussianBlur(input.texCoord);
  let dimColor = (textureSample(prevTexture, bloomSampler, input.texCoord) * bloom.dim);
  return (blurColor + dimColor);
}
