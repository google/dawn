bug/fxc/gradient_in_varying_loop/1112.wgsl:23:33 warning: 'textureSample' must only be called from uniform control flow
        let sampleDepth : f32 = textureSample(depthTexture, Sampler, offset.xy).r;
                                ^^^^^^^^^^^^^

bug/fxc/gradient_in_varying_loop/1112.wgsl:18:28 note: control flow depends on non-uniform value
        if (offset.x < 0.0 || offset.y < 0.0 || offset.x > 1.0 || offset.y > 1.0) {
                           ^^

bug/fxc/gradient_in_varying_loop/1112.wgsl:8:29 note: return value of 'textureSample' may be non-uniform
    let random: vec3<f32> = textureSample(randomTexture, Sampler, vUV).rgb;
                            ^^^^^^^^^^^^^

#version 310 es
precision mediump float;

layout(location = 0) in vec2 vUV_1;
layout(location = 0) out vec4 value;
uniform highp sampler2D randomTexture_Sampler;
uniform highp sampler2D depthTexture_Sampler;

vec4 tint_symbol(vec2 vUV) {
  vec3 random = texture(randomTexture_Sampler, vUV).rgb;
  int i = 0;
  while (true) {
    if ((i < 1)) {
    } else {
      break;
    }
    vec3 offset = vec3(random.x);
    bool tint_tmp_2 = (offset.x < 0.0f);
    if (!tint_tmp_2) {
      tint_tmp_2 = (offset.y < 0.0f);
    }
    bool tint_tmp_1 = (tint_tmp_2);
    if (!tint_tmp_1) {
      tint_tmp_1 = (offset.x > 1.0f);
    }
    bool tint_tmp = (tint_tmp_1);
    if (!tint_tmp) {
      tint_tmp = (offset.y > 1.0f);
    }
    if ((tint_tmp)) {
      i = (i + 1);
      continue;
    }
    float sampleDepth = texture(depthTexture_Sampler, offset.xy).r;
    i = (i + 1);
  }
  return vec4(1.0f);
}

void main() {
  vec4 inner_result = tint_symbol(vUV_1);
  value = inner_result;
  return;
}
