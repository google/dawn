#version 310 es
precision highp float;
precision highp int;

layout(location = 0) in vec2 vUV_1;
layout(location = 0) out vec4 value;
uniform highp sampler2D randomTexture_Sampler;

vec4 tint_symbol(vec2 vUV) {
  vec4 tint_symbol_1 = texture(randomTexture_Sampler, vUV);
  vec3 random = tint_symbol_1.rgb;
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
    float sampleDepth = 0.0f;
    i = (i + 1);
  }
  return vec4(1.0f);
}

void main() {
  vec4 inner_result = tint_symbol(vUV_1);
  value = inner_result;
  return;
}
