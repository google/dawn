#version 310 es
precision highp float;
precision highp int;

uniform highp sampler2D randomTexture_Sampler;
layout(location = 0) in vec2 tint_symbol_loc0_Input;
layout(location = 0) out vec4 tint_symbol_loc0_Output;
vec4 tint_symbol_inner(vec2 vUV) {
  vec3 random = texture(randomTexture_Sampler, vUV).xyz;
  int i = 0;
  {
    while(true) {
      if ((i < 1)) {
      } else {
        break;
      }
      vec3 offset = vec3(random[0u]);
      bool v = false;
      if ((offset[0u] < 0.0f)) {
        v = true;
      } else {
        v = (offset[1u] < 0.0f);
      }
      bool v_1 = false;
      if (v) {
        v_1 = true;
      } else {
        v_1 = (offset[0u] > 1.0f);
      }
      bool v_2 = false;
      if (v_1) {
        v_2 = true;
      } else {
        v_2 = (offset[1u] > 1.0f);
      }
      if (v_2) {
        i = (i + 1);
        {
        }
        continue;
      }
      float sampleDepth = 0.0f;
      i = (i + 1);
      {
      }
      continue;
    }
  }
  return vec4(1.0f);
}
void main() {
  tint_symbol_loc0_Output = tint_symbol_inner(tint_symbol_loc0_Input);
}
