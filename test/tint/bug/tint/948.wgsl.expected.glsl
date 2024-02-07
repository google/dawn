#version 310 es
precision highp float;
precision highp int;

float tint_float_modulo(float lhs, float rhs) {
  return (lhs - rhs * trunc(lhs / rhs));
}


layout(location = 2) in vec2 tUV_param_1;
layout(location = 5) in vec2 tileID_1_param_1;
layout(location = 4) in vec2 levelUnits_param_1;
layout(location = 3) in vec2 stageUnits_1_param_1;
layout(location = 0) in vec3 vPosition_param_1;
layout(location = 1) in vec2 vUV_param_1;
layout(location = 0) out vec4 glFragColor_1_1;
struct LeftOver {
  float time;
  uint padding;
  uint pad;
  uint pad_1;
  mat4 worldViewProjection;
  vec2 outputSize;
  vec2 stageSize;
  vec2 spriteMapSize;
  float stageScale;
  float spriteCount;
  vec3 colorMul;
  uint pad_2;
};

layout(binding = 9, std140) uniform x_20_block_ubo {
  LeftOver inner;
} x_20;

vec2 tUV = vec2(0.0f, 0.0f);
float mt = 0.0f;
vec4 glFragColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
vec2 tileID_1 = vec2(0.0f, 0.0f);
vec2 levelUnits = vec2(0.0f, 0.0f);
vec2 stageUnits_1 = vec2(0.0f, 0.0f);
vec3 vPosition = vec3(0.0f, 0.0f, 0.0f);
vec2 vUV = vec2(0.0f, 0.0f);
uniform highp sampler2D frameMapTexture_frameMapSampler;

mat4 getFrameData_f1_(inout float frameID) {
  float fX = 0.0f;
  float x_15 = frameID;
  float x_25 = x_20.inner.spriteCount;
  fX = (x_15 / x_25);
  float x_37 = fX;
  vec4 x_40 = texture(frameMapTexture_frameMapSampler, vec2(x_37, 0.0f), 0.0f);
  float x_44 = fX;
  vec4 x_47 = texture(frameMapTexture_frameMapSampler, vec2(x_44, 0.25f), 0.0f);
  float x_51 = fX;
  vec4 x_54 = texture(frameMapTexture_frameMapSampler, vec2(x_51, 0.5f), 0.0f);
  return mat4(vec4(x_40.x, x_40.y, x_40.z, x_40.w), vec4(x_47.x, x_47.y, x_47.z, x_47.w), vec4(x_54.x, x_54.y, x_54.z, x_54.w), vec4(0.0f));
}

uniform highp sampler2D tileMapsTexture1_tileMapsSampler;
uniform highp sampler2D tileMapsTexture0_tileMapsSampler;
uniform highp sampler2D animationMapTexture_animationMapSampler;
uniform highp sampler2D spriteSheetTexture_spriteSheetSampler;
void main_1() {
  vec4 color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  vec2 tileUV = vec2(0.0f, 0.0f);
  vec2 tileID = vec2(0.0f, 0.0f);
  vec2 sheetUnits = vec2(0.0f, 0.0f);
  float spriteUnits = 0.0f;
  vec2 stageUnits = vec2(0.0f, 0.0f);
  int i = 0;
  float frameID_1 = 0.0f;
  vec4 animationData = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  float f = 0.0f;
  mat4 frameData = mat4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float param = 0.0f;
  vec2 frameSize = vec2(0.0f, 0.0f);
  vec2 offset_1 = vec2(0.0f, 0.0f);
  vec2 ratio = vec2(0.0f, 0.0f);
  vec4 nc = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  float alpha = 0.0f;
  vec3 mixed = vec3(0.0f, 0.0f, 0.0f);
  color = vec4(0.0f);
  vec2 x_86 = tUV;
  tileUV = fract(x_86);
  float x_91 = tileUV.y;
  tileUV.y = (1.0f - x_91);
  vec2 x_95 = tUV;
  tileID = floor(x_95);
  vec2 x_101 = x_20.inner.spriteMapSize;
  sheetUnits = (vec2(1.0f) / x_101);
  float x_106 = x_20.inner.spriteCount;
  spriteUnits = (1.0f / x_106);
  vec2 x_111 = x_20.inner.stageSize;
  stageUnits = (vec2(1.0f) / x_111);
  i = 0;
  while (true) {
    int x_122 = i;
    if ((x_122 < 2)) {
    } else {
      break;
    }
    int x_126 = i;
    switch(x_126) {
      case 1: {
        vec2 x_150 = tileID;
        vec2 x_154 = x_20.inner.stageSize;
        vec4 x_156 = texture(tileMapsTexture1_tileMapsSampler, ((x_150 + vec2(0.5f)) / x_154), 0.0f);
        frameID_1 = x_156.x;
        break;
      }
      case 0: {
        vec2 x_136 = tileID;
        vec2 x_140 = x_20.inner.stageSize;
        vec4 x_142 = texture(tileMapsTexture0_tileMapsSampler, ((x_136 + vec2(0.5f)) / x_140), 0.0f);
        frameID_1 = x_142.x;
        break;
      }
      default: {
        break;
      }
    }
    float x_166 = frameID_1;
    float x_169 = x_20.inner.spriteCount;
    vec4 x_172 = texture(animationMapTexture_animationMapSampler, vec2(((x_166 + 0.5f) / x_169), 0.0f), 0.0f);
    animationData = x_172;
    float x_174 = animationData.y;
    if ((x_174 > 0.0f)) {
      float x_181 = x_20.inner.time;
      float x_184 = animationData.z;
      mt = tint_float_modulo((x_181 * x_184), 1.0f);
      f = 0.0f;
      while (true) {
        float x_193 = f;
        if ((x_193 < 8.0f)) {
        } else {
          break;
        }
        float x_197 = animationData.y;
        float x_198 = mt;
        if ((x_197 > x_198)) {
          float x_203 = animationData.x;
          frameID_1 = x_203;
          break;
        }
        float x_208 = frameID_1;
        float x_211 = x_20.inner.spriteCount;
        float x_214 = f;
        vec4 x_217 = vec4(0.0f);
        animationData = x_217;
        {
          float x_218 = f;
          f = (x_218 + 1.0f);
        }
      }
    }
    float x_222 = frameID_1;
    param = (x_222 + 0.5f);
    mat4 x_225 = getFrameData_f1_(param);
    frameData = x_225;
    vec4 x_228 = frameData[0];
    vec2 x_231 = x_20.inner.spriteMapSize;
    frameSize = (vec2(x_228.w, x_228.z) / x_231);
    vec4 x_235 = frameData[0];
    vec2 x_237 = sheetUnits;
    offset_1 = (vec2(x_235.x, x_235.y) * x_237);
    vec4 x_241 = frameData[2];
    vec4 x_244 = frameData[0];
    ratio = (vec2(x_241.x, x_241.y) / vec2(x_244.w, x_244.z));
    float x_248 = frameData[2].z;
    if ((x_248 == 1.0f)) {
      vec2 x_252 = tileUV;
      tileUV = vec2(x_252.y, x_252.x);
    }
    int x_254 = i;
    if ((x_254 == 0)) {
      vec2 x_263 = tileUV;
      vec2 x_264 = frameSize;
      vec2 x_266 = offset_1;
      vec4 x_268 = texture(spriteSheetTexture_spriteSheetSampler, ((x_263 * x_264) + x_266));
      color = x_268;
    } else {
      vec2 x_274 = tileUV;
      vec2 x_275 = frameSize;
      vec2 x_277 = offset_1;
      vec4 x_279 = texture(spriteSheetTexture_spriteSheetSampler, ((x_274 * x_275) + x_277));
      nc = x_279;
      float x_283 = color.w;
      float x_285 = nc.w;
      alpha = min((x_283 + x_285), 1.0f);
      vec4 x_290 = color;
      vec4 x_292 = nc;
      float x_295 = nc.w;
      mixed = mix(vec3(x_290.x, x_290.y, x_290.z), vec3(x_292.x, x_292.y, x_292.z), vec3(x_295, x_295, x_295));
      vec3 x_298 = mixed;
      float x_299 = alpha;
      color = vec4(x_298.x, x_298.y, x_298.z, x_299);
    }
    {
      int x_304 = i;
      i = (x_304 + 1);
    }
  }
  vec3 x_310 = x_20.inner.colorMul;
  vec4 x_311 = color;
  vec3 x_313 = (vec3(x_311.x, x_311.y, x_311.z) * x_310);
  vec4 x_314 = color;
  color = vec4(x_313.x, x_313.y, x_313.z, x_314.w);
  vec4 x_318 = color;
  glFragColor = x_318;
  return;
}

struct main_out {
  vec4 glFragColor_1;
};

main_out tint_symbol(vec2 tUV_param, vec2 tileID_1_param, vec2 levelUnits_param, vec2 stageUnits_1_param, vec3 vPosition_param, vec2 vUV_param) {
  tUV = tUV_param;
  tileID_1 = tileID_1_param;
  levelUnits = levelUnits_param;
  stageUnits_1 = stageUnits_1_param;
  vPosition = vPosition_param;
  vUV = vUV_param;
  main_1();
  main_out tint_symbol_1 = main_out(glFragColor);
  return tint_symbol_1;
}

void main() {
  main_out inner_result = tint_symbol(tUV_param_1, tileID_1_param_1, levelUnits_param_1, stageUnits_1_param_1, vPosition_param_1, vUV_param_1);
  glFragColor_1_1 = inner_result.glFragColor_1;
  return;
}
