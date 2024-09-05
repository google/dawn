#version 310 es
precision highp float;
precision highp int;


struct Scene {
  vec4 vEyePosition;
};

struct Material {
  vec4 vDiffuseColor;
  vec3 vAmbientColor;
  float placeholder;
  vec3 vEmissiveColor;
  float placeholder2;
};

struct Mesh {
  float visibility;
};

struct main_out {
  vec4 glFragColor_1;
};

float fClipDistance3 = 0.0f;
float fClipDistance4 = 0.0f;
layout(binding = 0, std140)
uniform tint_symbol_2_1_ubo {
  Scene tint_symbol_1;
} v;
layout(binding = 1, std140)
uniform tint_symbol_4_1_ubo {
  Material tint_symbol_3;
} v_1;
layout(binding = 2, std140)
uniform tint_symbol_6_1_ubo {
  Mesh tint_symbol_5;
} v_2;
vec4 glFragColor = vec4(0.0f);
bool continue_execution = true;
layout(location = 2) in float tint_symbol_loc2_Input;
layout(location = 3) in float tint_symbol_loc3_Input;
layout(location = 0) out vec4 tint_symbol_loc0_Output;
void main_1() {
  vec3 viewDirectionW = vec3(0.0f);
  vec4 baseColor = vec4(0.0f);
  vec3 diffuseColor = vec3(0.0f);
  float alpha = 0.0f;
  vec3 normalW = vec3(0.0f);
  vec2 uvOffset = vec2(0.0f);
  vec3 baseAmbientColor = vec3(0.0f);
  float glossiness = 0.0f;
  vec3 diffuseBase = vec3(0.0f);
  float shadow = 0.0f;
  vec4 refractionColor = vec4(0.0f);
  vec4 reflectionColor = vec4(0.0f);
  vec3 emissiveColor = vec3(0.0f);
  vec3 finalDiffuse = vec3(0.0f);
  vec3 finalSpecular = vec3(0.0f);
  vec4 color = vec4(0.0f);
  float x_9 = fClipDistance3;
  if ((x_9 > 0.0f)) {
    continue_execution = false;
  }
  float x_17 = fClipDistance4;
  if ((x_17 > 0.0f)) {
    continue_execution = false;
  }
  vec4 x_34 = v.tint_symbol_1.vEyePosition;
  vec3 x_38 = vec3(0.0f);
  viewDirectionW = normalize((vec3(x_34[0u], x_34[1u], x_34[2u]) - x_38));
  baseColor = vec4(1.0f);
  vec4 x_52 = v_1.tint_symbol_3.vDiffuseColor;
  diffuseColor = vec3(x_52[0u], x_52[1u], x_52[2u]);
  float x_60 = v_1.tint_symbol_3.vDiffuseColor.w;
  alpha = x_60;
  vec3 x_62 = vec3(0.0f);
  vec3 x_64 = vec3(0.0f);
  uvOffset = vec2(0.0f);
  vec4 x_74 = vec4(0.0f);
  vec4 x_76 = baseColor;
  vec3 v_3 = vec3(x_76[0u], x_76[1u], x_76[2u]);
  vec3 x_78 = (v_3 * vec3(x_74[0u], x_74[1u], x_74[2u]));
  vec4 x_79 = baseColor;
  baseColor = vec4(x_78[0u], x_78[1u], x_78[2u], x_79[3u]);
  baseAmbientColor = vec3(1.0f);
  glossiness = 0.0f;
  diffuseBase = vec3(0.0f);
  shadow = 1.0f;
  refractionColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
  reflectionColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
  vec3 x_94 = v_1.tint_symbol_3.vEmissiveColor;
  emissiveColor = x_94;
  vec3 x_96 = diffuseBase;
  vec3 x_97 = diffuseColor;
  vec3 x_99 = emissiveColor;
  vec3 x_103 = v_1.tint_symbol_3.vAmbientColor;
  vec4 x_108 = baseColor;
  vec3 v_4 = clamp((((x_96 * x_97) + x_99) + x_103), vec3(0.0f), vec3(1.0f));
  finalDiffuse = (v_4 * vec3(x_108[0u], x_108[1u], x_108[2u]));
  finalSpecular = vec3(0.0f);
  vec3 x_113 = finalDiffuse;
  vec3 x_114 = baseAmbientColor;
  vec3 x_116 = finalSpecular;
  vec4 x_118 = reflectionColor;
  vec4 x_121 = refractionColor;
  vec3 v_5 = (((x_113 * x_114) + x_116) + vec3(x_118[0u], x_118[1u], x_118[2u]));
  vec3 x_123 = (v_5 + vec3(x_121[0u], x_121[1u], x_121[2u]));
  float x_124 = alpha;
  color = vec4(x_123[0u], x_123[1u], x_123[2u], x_124);
  vec4 x_129 = color;
  vec3 x_132 = max(vec3(x_129[0u], x_129[1u], x_129[2u]), vec3(0.0f));
  vec4 x_133 = color;
  color = vec4(x_132[0u], x_132[1u], x_132[2u], x_133[3u]);
  float x_140 = v_2.tint_symbol_5.visibility;
  float x_142 = color.w;
  color[3u] = (x_142 * x_140);
  vec4 x_147 = color;
  glFragColor = x_147;
}
main_out tint_symbol_inner(float fClipDistance3_param, float fClipDistance4_param) {
  fClipDistance3 = fClipDistance3_param;
  fClipDistance4 = fClipDistance4_param;
  main_1();
  main_out v_6 = main_out(glFragColor);
  if (!(continue_execution)) {
    discard;
  }
  return v_6;
}
void main() {
  tint_symbol_loc0_Output = tint_symbol_inner(tint_symbol_loc2_Input, tint_symbol_loc3_Input).glFragColor_1;
}
