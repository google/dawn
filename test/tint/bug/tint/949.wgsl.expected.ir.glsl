#version 310 es
precision highp float;
precision highp int;


struct LeftOver {
  mat4 u_World;
  mat4 u_ViewProjection;
  float u_bumpStrength;
  uint padding;
  vec3 u_cameraPosition;
  float u_parallaxScale;
  float textureInfoName;
  uint padding_1;
  vec2 tangentSpaceParameter0;
};

struct Light0 {
  vec4 vLightData;
  vec4 vLightDiffuse;
  vec4 vLightSpecular;
  vec3 vLightGround;
  uint padding_2;
  vec4 shadowsInfo;
  vec2 depthValues;
};

struct lightingInfo {
  vec3 diffuse;
  vec3 specular;
};

struct main_out {
  vec4 glFragColor_1;
};

float u_Float = 0.0f;
vec3 u_Color = vec3(0.0f);
vec2 vMainuv = vec2(0.0f);
layout(binding = 6, std140)
uniform tint_symbol_4_1_ubo {
  LeftOver tint_symbol_3;
} v;
vec4 v_output1 = vec4(0.0f);
bool tint_symbol = false;
vec2 v_uv = vec2(0.0f);
vec4 v_output2 = vec4(0.0f);
layout(binding = 5, std140)
uniform tint_symbol_6_1_ubo {
  Light0 tint_symbol_5;
} v_1;
vec4 glFragColor = vec4(0.0f);
uniform highp sampler2D TextureSamplerTexture_TextureSamplerSampler;
uniform highp sampler2D TextureSampler1Texture_TextureSampler1Sampler;
layout(location = 1) in vec2 tint_symbol_1_loc1_Input;
layout(location = 0) in vec4 tint_symbol_1_loc0_Input;
layout(location = 3) in vec2 tint_symbol_1_loc3_Input;
layout(location = 2) in vec4 tint_symbol_1_loc2_Input;
layout(location = 0) out vec4 tint_symbol_1_loc0_Output;
mat3 cotangent_frame_vf3_vf3_vf2_vf2_(inout vec3 normal_1, inout vec3 p, inout vec2 uv, inout vec2 tangentSpaceParams) {
  vec3 dp1 = vec3(0.0f);
  vec3 dp2 = vec3(0.0f);
  vec2 duv1 = vec2(0.0f);
  vec2 duv2 = vec2(0.0f);
  vec3 dp2perp = vec3(0.0f);
  vec3 dp1perp = vec3(0.0f);
  vec3 tangent = vec3(0.0f);
  vec3 bitangent = vec3(0.0f);
  float invmax = 0.0f;
  vec3 x_133 = p;
  dp1 = dFdx(x_133);
  vec3 x_136 = p;
  dp2 = dFdy(x_136);
  vec2 x_139 = uv;
  duv1 = dFdx(x_139);
  vec2 x_142 = uv;
  duv2 = dFdy(x_142);
  vec3 x_145 = dp2;
  vec3 x_146 = normal_1;
  dp2perp = cross(x_145, x_146);
  vec3 x_149 = normal_1;
  vec3 x_150 = dp1;
  dp1perp = cross(x_149, x_150);
  vec3 x_153 = dp2perp;
  float x_155 = duv1.x;
  vec3 x_157 = dp1perp;
  float x_159 = duv2.x;
  tangent = ((x_153 * x_155) + (x_157 * x_159));
  vec3 x_163 = dp2perp;
  float x_165 = duv1.y;
  vec3 x_167 = dp1perp;
  float x_169 = duv2.y;
  bitangent = ((x_163 * x_165) + (x_167 * x_169));
  float x_173 = tangentSpaceParams.x;
  vec3 x_174 = tangent;
  tangent = (x_174 * x_173);
  float x_177 = tangentSpaceParams.y;
  vec3 x_178 = bitangent;
  bitangent = (x_178 * x_177);
  vec3 x_181 = tangent;
  vec3 x_182 = tangent;
  vec3 x_184 = bitangent;
  vec3 x_185 = bitangent;
  float v_2 = dot(x_181, x_182);
  invmax = inversesqrt(max(v_2, dot(x_184, x_185)));
  vec3 x_189 = tangent;
  float x_190 = invmax;
  vec3 x_191 = (x_189 * x_190);
  vec3 x_192 = bitangent;
  float x_193 = invmax;
  vec3 x_194 = (x_192 * x_193);
  vec3 x_195 = normal_1;
  vec3 v_3 = vec3(x_191[0u], x_191[1u], x_191[2u]);
  vec3 v_4 = vec3(x_194[0u], x_194[1u], x_194[2u]);
  return mat3(v_3, v_4, vec3(x_195[0u], x_195[1u], x_195[2u]));
}
mat3 transposeMat3_mf33_(inout mat3 inMatrix) {
  vec3 i0 = vec3(0.0f);
  vec3 i1 = vec3(0.0f);
  vec3 i2 = vec3(0.0f);
  mat3 outMatrix = mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f));
  vec3 x_60 = inMatrix[0];
  i0 = x_60;
  vec3 x_64 = inMatrix[1];
  i1 = x_64;
  vec3 x_68 = inMatrix[2];
  i2 = x_68;
  float x_73 = i0.x;
  float x_75 = i1.x;
  float x_77 = i2.x;
  vec3 x_78 = vec3(x_73, x_75, x_77);
  float x_81 = i0.y;
  float x_83 = i1.y;
  float x_85 = i2.y;
  vec3 x_86 = vec3(x_81, x_83, x_85);
  float x_89 = i0.z;
  float x_91 = i1.z;
  float x_93 = i2.z;
  vec3 x_94 = vec3(x_89, x_91, x_93);
  vec3 v_5 = vec3(x_78[0u], x_78[1u], x_78[2u]);
  vec3 v_6 = vec3(x_86[0u], x_86[1u], x_86[2u]);
  outMatrix = mat3(v_5, v_6, vec3(x_94[0u], x_94[1u], x_94[2u]));
  mat3 x_110 = outMatrix;
  return x_110;
}
vec3 perturbNormalBase_mf33_vf3_f1_(inout mat3 cotangentFrame, inout vec3 normal, inout float scale) {
  mat3 x_113 = cotangentFrame;
  vec3 x_114 = normal;
  return normalize((x_113 * x_114));
}
vec3 perturbNormal_mf33_vf3_f1_(inout mat3 cotangentFrame_1, inout vec3 textureSample, inout float scale_1) {
  mat3 param = mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f));
  vec3 param_1 = vec3(0.0f);
  float param_2 = 0.0f;
  vec3 x_119 = textureSample;
  mat3 x_125 = cotangentFrame_1;
  param = x_125;
  param_1 = ((x_119 * 2.0f) - vec3(1.0f));
  float x_128 = scale_1;
  param_2 = x_128;
  vec3 x_129 = perturbNormalBase_mf33_vf3_f1_(param, param_1, param_2);
  return x_129;
}
lightingInfo computeHemisphericLighting_vf3_vf3_vf4_vf3_vf3_vf3_f1_(inout vec3 viewDirectionW, inout vec3 vNormal, inout vec4 lightData, inout vec3 diffuseColor, inout vec3 specularColor, inout vec3 groundColor, inout float glossiness) {
  float ndl = 0.0f;
  lightingInfo result = lightingInfo(vec3(0.0f), vec3(0.0f));
  vec3 angleW = vec3(0.0f);
  float specComp = 0.0f;
  vec3 x_212 = vNormal;
  vec4 x_213 = lightData;
  ndl = ((dot(x_212, vec3(x_213[0u], x_213[1u], x_213[2u])) * 0.5f) + 0.5f);
  vec3 x_220 = groundColor;
  vec3 x_221 = diffuseColor;
  float x_222 = ndl;
  result.diffuse = mix(x_220, x_221, vec3(x_222, x_222, x_222));
  vec3 x_227 = viewDirectionW;
  vec4 x_228 = lightData;
  angleW = normalize((x_227 + vec3(x_228[0u], x_228[1u], x_228[2u])));
  vec3 x_233 = vNormal;
  vec3 x_234 = angleW;
  specComp = max(0.0f, dot(x_233, x_234));
  float x_237 = specComp;
  float x_238 = glossiness;
  specComp = pow(x_237, max(1.0f, x_238));
  float x_241 = specComp;
  vec3 x_242 = specularColor;
  result.specular = (x_242 * x_241);
  lightingInfo x_245 = result;
  return x_245;
}
void main_1() {
  vec4 tempTextureRead = vec4(0.0f);
  vec3 rgb = vec3(0.0f);
  vec3 output5 = vec3(0.0f);
  vec4 output4 = vec4(0.0f);
  vec2 uvOffset = vec2(0.0f);
  float normalScale = 0.0f;
  vec2 TBNUV = vec2(0.0f);
  vec2 x_299 = vec2(0.0f);
  mat3 TBN = mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f));
  vec3 param_3 = vec3(0.0f);
  vec3 param_4 = vec3(0.0f);
  vec2 param_5 = vec2(0.0f);
  vec2 param_6 = vec2(0.0f);
  mat3 invTBN = mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f));
  mat3 param_7 = mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f));
  float parallaxLimit = 0.0f;
  vec2 vOffsetDir = vec2(0.0f);
  vec2 vMaxOffset = vec2(0.0f);
  float numSamples = 0.0f;
  float stepSize = 0.0f;
  float currRayHeight = 0.0f;
  vec2 vCurrOffset = vec2(0.0f);
  vec2 vLastOffset = vec2(0.0f);
  float lastSampledHeight = 0.0f;
  float currSampledHeight = 0.0f;
  int i = 0;
  float delta1 = 0.0f;
  float delta2 = 0.0f;
  float ratio = 0.0f;
  vec2 parallaxOcclusion_0 = vec2(0.0f);
  mat3 param_8 = mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f));
  vec3 param_9 = vec3(0.0f);
  float param_10 = 0.0f;
  vec2 output6 = vec2(0.0f);
  vec4 tempTextureRead1 = vec4(0.0f);
  vec3 rgb1 = vec3(0.0f);
  vec3 viewDirectionW_1 = vec3(0.0f);
  float shadow = 0.0f;
  float glossiness_1 = 0.0f;
  vec3 diffuseBase = vec3(0.0f);
  vec3 specularBase = vec3(0.0f);
  vec3 normalW = vec3(0.0f);
  lightingInfo info = lightingInfo(vec3(0.0f), vec3(0.0f));
  vec3 param_11 = vec3(0.0f);
  vec3 param_12 = vec3(0.0f);
  vec4 param_13 = vec4(0.0f);
  vec3 param_14 = vec3(0.0f);
  vec3 param_15 = vec3(0.0f);
  vec3 param_16 = vec3(0.0f);
  float param_17 = 0.0f;
  vec3 diffuseOutput = vec3(0.0f);
  vec3 specularOutput = vec3(0.0f);
  vec3 output3 = vec3(0.0f);
  u_Float = 100.0f;
  u_Color = vec3(0.5f);
  vec2 x_261 = vMainuv;
  vec4 x_262 = texture(TextureSamplerTexture_TextureSamplerSampler, x_261);
  tempTextureRead = x_262;
  vec4 x_264 = tempTextureRead;
  float x_273 = v.tint_symbol_3.textureInfoName;
  rgb = (vec3(x_264[0u], x_264[1u], x_264[2u]) * x_273);
  vec3 x_279 = v.tint_symbol_3.u_cameraPosition;
  vec4 x_282 = v_output1;
  output5 = normalize((x_279 - vec3(x_282[0u], x_282[1u], x_282[2u])));
  output4 = vec4(0.0f);
  uvOffset = vec2(0.0f);
  float x_292 = v.tint_symbol_3.u_bumpStrength;
  normalScale = (1.0f / x_292);
  bool x_298 = tint_symbol;
  if (x_298) {
    vec2 x_303 = v_uv;
    x_299 = x_303;
  } else {
    vec2 x_305 = v_uv;
    x_299 = -(x_305);
  }
  vec2 x_307 = x_299;
  TBNUV = x_307;
  vec4 x_310 = v_output2;
  float x_312 = normalScale;
  param_3 = (vec3(x_310[0u], x_310[1u], x_310[2u]) * x_312);
  vec4 x_317 = v_output1;
  param_4 = vec3(x_317[0u], x_317[1u], x_317[2u]);
  vec2 x_320 = TBNUV;
  param_5 = x_320;
  vec2 x_324 = v.tint_symbol_3.tangentSpaceParameter0;
  param_6 = x_324;
  mat3 x_325 = cotangent_frame_vf3_vf3_vf2_vf2_(param_3, param_4, param_5, param_6);
  TBN = x_325;
  mat3 x_328 = TBN;
  param_7 = x_328;
  mat3 x_329 = transposeMat3_mf33_(param_7);
  invTBN = x_329;
  mat3 x_331 = invTBN;
  vec3 x_332 = output5;
  vec3 x_334 = (x_331 * -(x_332));
  mat3 x_337 = invTBN;
  vec3 x_338 = output5;
  parallaxLimit = (length(vec2(x_334[0u], x_334[1u])) / (x_337 * -(x_338))[2u]);
  float x_345 = v.tint_symbol_3.u_parallaxScale;
  float x_346 = parallaxLimit;
  parallaxLimit = (x_346 * x_345);
  mat3 x_349 = invTBN;
  vec3 x_350 = output5;
  vec3 x_352 = (x_349 * -(x_350));
  vOffsetDir = normalize(vec2(x_352[0u], x_352[1u]));
  vec2 x_356 = vOffsetDir;
  float x_357 = parallaxLimit;
  vMaxOffset = (x_356 * x_357);
  mat3 x_361 = invTBN;
  vec3 x_362 = output5;
  mat3 x_365 = invTBN;
  vec4 x_366 = v_output2;
  numSamples = (15.0f + (dot((x_361 * -(x_362)), (x_365 * vec3(x_366[0u], x_366[1u], x_366[2u]))) * -11.0f));
  float x_374 = numSamples;
  stepSize = (1.0f / x_374);
  currRayHeight = 1.0f;
  vCurrOffset = vec2(0.0f);
  vLastOffset = vec2(0.0f);
  lastSampledHeight = 1.0f;
  currSampledHeight = 1.0f;
  i = 0;
  {
    while(true) {
      int x_388 = i;
      if ((x_388 < 15)) {
      } else {
        break;
      }
      vec2 x_394 = v_uv;
      vec2 x_395 = vCurrOffset;
      vec4 x_397 = vec4(0.0f);
      currSampledHeight = x_397[3u];
      float x_400 = currSampledHeight;
      float x_401 = currRayHeight;
      if ((x_400 > x_401)) {
        float x_406 = currSampledHeight;
        float x_407 = currRayHeight;
        delta1 = (x_406 - x_407);
        float x_410 = currRayHeight;
        float x_411 = stepSize;
        float x_413 = lastSampledHeight;
        delta2 = ((x_410 + x_411) - x_413);
        float x_416 = delta1;
        float x_417 = delta1;
        float x_418 = delta2;
        ratio = (x_416 / (x_417 + x_418));
        float x_421 = ratio;
        vec2 x_422 = vLastOffset;
        float x_424 = ratio;
        vec2 x_426 = vCurrOffset;
        vCurrOffset = ((x_422 * x_421) + (x_426 * (1.0f - x_424)));
        break;
      } else {
        float x_431 = stepSize;
        float x_432 = currRayHeight;
        currRayHeight = (x_432 - x_431);
        vec2 x_434 = vCurrOffset;
        vLastOffset = x_434;
        float x_435 = stepSize;
        vec2 x_436 = vMaxOffset;
        vec2 x_438 = vCurrOffset;
        vCurrOffset = (x_438 + (x_436 * x_435));
        float x_440 = currSampledHeight;
        lastSampledHeight = x_440;
      }
      {
        int x_441 = i;
        i = (x_441 + 1);
      }
      continue;
    }
  }
  vec2 x_444 = vCurrOffset;
  parallaxOcclusion_0 = x_444;
  vec2 x_445 = parallaxOcclusion_0;
  uvOffset = x_445;
  vec2 x_449 = v_uv;
  vec2 x_450 = uvOffset;
  vec4 x_452 = texture(TextureSamplerTexture_TextureSamplerSampler, (x_449 + x_450));
  float x_454 = v.tint_symbol_3.u_bumpStrength;
  mat3 x_457 = TBN;
  param_8 = x_457;
  param_9 = vec3(x_452[0u], x_452[1u], x_452[2u]);
  param_10 = (1.0f / x_454);
  vec3 x_461 = perturbNormal_mf33_vf3_f1_(param_8, param_9, param_10);
  vec4 x_462 = output4;
  output4 = vec4(x_461[0u], x_461[1u], x_461[2u], x_462[3u]);
  vec2 x_465 = v_uv;
  vec2 x_466 = uvOffset;
  output6 = (x_465 + x_466);
  vec2 x_474 = output6;
  vec4 x_475 = texture(TextureSampler1Texture_TextureSampler1Sampler, x_474);
  tempTextureRead1 = x_475;
  vec4 x_477 = tempTextureRead1;
  rgb1 = vec3(x_477[0u], x_477[1u], x_477[2u]);
  vec3 x_481 = v.tint_symbol_3.u_cameraPosition;
  vec4 x_482 = v_output1;
  viewDirectionW_1 = normalize((x_481 - vec3(x_482[0u], x_482[1u], x_482[2u])));
  shadow = 1.0f;
  float x_488 = u_Float;
  glossiness_1 = (1.0f * x_488);
  diffuseBase = vec3(0.0f);
  specularBase = vec3(0.0f);
  vec4 x_494 = output4;
  normalW = vec3(x_494[0u], x_494[1u], x_494[2u]);
  vec3 x_501 = viewDirectionW_1;
  param_11 = x_501;
  vec3 x_503 = normalW;
  param_12 = x_503;
  vec4 x_507 = v_1.tint_symbol_5.vLightData;
  param_13 = x_507;
  vec4 x_510 = v_1.tint_symbol_5.vLightDiffuse;
  param_14 = vec3(x_510[0u], x_510[1u], x_510[2u]);
  vec4 x_514 = v_1.tint_symbol_5.vLightSpecular;
  param_15 = vec3(x_514[0u], x_514[1u], x_514[2u]);
  vec3 x_518 = v_1.tint_symbol_5.vLightGround;
  param_16 = x_518;
  float x_520 = glossiness_1;
  param_17 = x_520;
  lightingInfo x_521 = computeHemisphericLighting_vf3_vf3_vf4_vf3_vf3_vf3_f1_(param_11, param_12, param_13, param_14, param_15, param_16, param_17);
  info = x_521;
  shadow = 1.0f;
  vec3 x_523 = info.diffuse;
  float x_524 = shadow;
  vec3 x_526 = diffuseBase;
  diffuseBase = (x_526 + (x_523 * x_524));
  vec3 x_529 = info.specular;
  float x_530 = shadow;
  vec3 x_532 = specularBase;
  specularBase = (x_532 + (x_529 * x_530));
  vec3 x_535 = diffuseBase;
  vec3 x_536 = rgb1;
  diffuseOutput = (x_535 * x_536);
  vec3 x_539 = specularBase;
  vec3 x_540 = u_Color;
  specularOutput = (x_539 * x_540);
  vec3 x_543 = diffuseOutput;
  vec3 x_544 = specularOutput;
  output3 = (x_543 + x_544);
  vec3 x_548 = output3;
  glFragColor = vec4(x_548[0u], x_548[1u], x_548[2u], 1.0f);
}
main_out tint_symbol_1_inner(vec2 vMainuv_param, vec4 v_output1_param, bool tint_symbol_2, vec2 v_uv_param, vec4 v_output2_param) {
  vMainuv = vMainuv_param;
  v_output1 = v_output1_param;
  tint_symbol = tint_symbol_2;
  v_uv = v_uv_param;
  v_output2 = v_output2_param;
  main_1();
  return main_out(glFragColor);
}
void main() {
  tint_symbol_1_loc0_Output = tint_symbol_1_inner(tint_symbol_1_loc1_Input, tint_symbol_1_loc0_Input, gl_FrontFacing, tint_symbol_1_loc3_Input, tint_symbol_1_loc2_Input).glFragColor_1;
}
