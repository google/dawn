bug/fxc/gradient_in_varying_loop/1112.wgsl:23:33 warning: 'textureSample' must only be called from uniform control flow
        let sampleDepth : f32 = textureSample(depthTexture, Sampler, offset.xy).r;
                                ^^^^^^^^^^^^^

bug/fxc/gradient_in_varying_loop/1112.wgsl:18:28 note: control flow depends on non-uniform value
        if (offset.x < 0.0 || offset.y < 0.0 || offset.x > 1.0 || offset.y > 1.0) {
                           ^^

bug/fxc/gradient_in_varying_loop/1112.wgsl:8:29 note: return value of 'textureSample' may be non-uniform
    let random: vec3<f32> = textureSample(randomTexture, Sampler, vUV).rgb;
                            ^^^^^^^^^^^^^

@group(0) @binding(0) var Sampler : sampler;

@group(0) @binding(1) var randomTexture : texture_2d<f32>;

@group(0) @binding(2) var depthTexture : texture_2d<f32>;

@fragment
fn main(@location(0) vUV : vec2<f32>) -> @location(0) vec4<f32> {
  let random : vec3<f32> = textureSample(randomTexture, Sampler, vUV).rgb;
  var i = 0;
  loop {
    if ((i < 1)) {
    } else {
      break;
    }
    let offset : vec3<f32> = vec3<f32>(random.x);
    if (((((offset.x < 0.0) || (offset.y < 0.0)) || (offset.x > 1.0)) || (offset.y > 1.0))) {
      i = (i + 1);
      continue;
    }
    let sampleDepth : f32 = textureSample(depthTexture, Sampler, offset.xy).r;
    i = (i + 1);
  }
  return vec4<f32>(1.0);
}
