diagnostic(off, derivative_uniformity);

var<private> u_xlat0 : vec3<f32>;

var<private> u_xlatb26 : vec2<bool>;

struct UnityPerDraw_1 {
  unity_ObjectToWorld : mat4x4<f32>,
  unity_WorldToObject : mat4x4<f32>,
  unity_LODFade : vec4<f32>,
  unity_WorldTransformParams : vec4<f32>,
  unity_RenderingLayer : vec4<f32>,
  unity_LightData : vec4<f32>,
  unity_LightIndices : array<vec4<f32>, 2u>,
  unity_ProbesOcclusion : vec4<f32>,
  unity_SpecCube0_HDR : vec4<f32>,
  unity_SpecCube1_HDR : vec4<f32>,
  unity_SpecCube0_BoxMax : vec4<f32>,
  unity_SpecCube0_BoxMin : vec4<f32>,
  unity_SpecCube0_ProbePosition : vec4<f32>,
  unity_SpecCube1_BoxMax : vec4<f32>,
  unity_SpecCube1_BoxMin : vec4<f32>,
  unity_SpecCube1_ProbePosition : vec4<f32>,
  unity_LightmapST : vec4<f32>,
  unity_DynamicLightmapST : vec4<f32>,
  unity_SHAr : vec4<f32>,
  unity_SHAg : vec4<f32>,
  unity_SHAb : vec4<f32>,
  unity_SHBr : vec4<f32>,
  unity_SHBg : vec4<f32>,
  unity_SHBb : vec4<f32>,
  unity_SHC : vec4<f32>,
  unity_RendererBounds_Min : vec4<f32>,
  unity_RendererBounds_Max : vec4<f32>,
  unity_MatrixPreviousM : mat4x4<f32>,
  unity_MatrixPreviousMI : mat4x4<f32>,
  unity_MotionVectorsParams : vec4<f32>,
  unity_SpriteColor : vec4<f32>,
  unity_SpriteProps : vec4<f32>,
}

@group(1u) @binding(2u) var<uniform> v : UnityPerDraw_1;

var<private> u_xlat26 : vec3<f32>;

var<private> u_xlat1 : vec3<f32>;

var<private> u_xlat2 : vec4<f32>;

var<private> u_xlat3 : vec4<f32>;

var<private> u_xlatb0 : bool;

struct PGlobals_1 {
  m : vec2<f32>,
  m_1 : f32,
  m_2 : vec4<f32>,
  m_3 : vec4<f32>,
  m_4 : vec4<f32>,
  m_5 : vec3<f32>,
  m_6 : vec4<f32>,
  m_7 : vec4<f32>,
  unity_OrthoParams : vec4<f32>,
  unity_FogParams : vec4<f32>,
  unity_FogColor : vec4<f32>,
  unity_MatrixV : mat4x4<f32>,
}

@group(1u) @binding(0u) var<uniform> v_1 : PGlobals_1;

var<private> u_xlat4 : vec3<f32>;

var<private> u_xlat79 : f32;

var<private> u_xlat5 : vec4<f32>;

var<private> u_xlat6 : vec4<f32>;

@group(0u) @binding(7u) var Texture2D_B222E8F : texture_2d<f32>;

@group(0u) @binding(14u) var samplerTexture2D_B222E8F : sampler;

var<private> u_xlat7 : vec3<f32>;

struct UnityPerMaterial {
  Texture2D_B222E8F_TexelSize : vec4<f32>,
  Color_C30C7CA3 : vec4<f32>,
  Texture2D_D9BFD5F1_TexelSize : vec4<f32>,
}

@group(1u) @binding(5u) var<uniform> v_2 : UnityPerMaterial;

var<private> u_xlat8 : vec4<f32>;

@group(0u) @binding(8u) var Texture2D_D9BFD5F1 : texture_2d<f32>;

@group(0u) @binding(15u) var samplerTexture2D_D9BFD5F1 : sampler;

var<private> u_xlat9 : vec4<f32>;

var<private> u_xlat34 : vec3<f32>;

struct LightShadows_1 {
  m_8 : array<mat4x4<f32>, 5u>,
  m_9 : vec4<f32>,
  m_10 : vec4<f32>,
  m_11 : vec4<f32>,
  m_12 : vec4<f32>,
  m_13 : vec4<f32>,
  m_14 : vec4<f32>,
  m_15 : vec4<f32>,
  m_16 : vec4<f32>,
  m_17 : vec4<f32>,
  m_18 : vec4<f32>,
  m_19 : vec4<f32>,
  m_20 : vec4<f32>,
  m_21 : vec4<f32>,
  m_22 : array<vec4<f32>, 32u>,
  m_23 : array<mat4x4<f32>, 32u>,
}

@group(1u) @binding(3u) var<uniform> v_3 : LightShadows_1;

var<private> u_xlat10 : vec4<f32>;

var<private> u_xlatb2 : vec4<bool>;

var<private> u_xlatu0 : u32;

var<private> u_xlati0 : i32;

var<private> u_xlatb79 : bool;

@group(0u) @binding(3u) var v_4 : texture_depth_2d;

@group(0u) @binding(12u) var sampler_LinearClampCompare : sampler_comparison;

var<private> u_xlatb80 : bool;

var<private> u_xlat55 : vec2<f32>;

var<private> u_xlat62 : vec2<f32>;

var<private> u_xlat11 : vec4<f32>;

var<private> u_xlat12 : vec4<f32>;

var<private> u_xlat13 : vec4<f32>;

var<private> u_xlat14 : vec4<f32>;

var<private> u_xlat15 : vec4<f32>;

var<private> u_xlat16 : vec4<f32>;

var<private> u_xlat80 : f32;

var<private> u_xlat29 : f32;

var<private> u_xlat35 : vec3<f32>;

var<private> u_xlat17 : vec4<f32>;

var<private> u_xlat18 : vec4<f32>;

var<private> u_xlat36 : vec2<f32>;

var<private> u_xlat68 : vec2<f32>;

var<private> u_xlat63 : vec2<f32>;

var<private> u_xlat19 : vec4<f32>;

var<private> u_xlat20 : vec4<f32>;

var<private> u_xlat21 : vec4<f32>;

var<private> u_xlat82 : f32;

var<private> u_xlatb3 : vec4<bool>;

var<private> u_xlatb29 : bool;

var<private> u_xlat27 : vec3<f32>;

var<private> u_xlatu5 : vec3<u32>;

var<private> u_xlatu55 : u32;

var<private> u_xlatu81 : u32;

var<private> u_xlati55 : i32;

var<private> u_xlat81 : f32;

var<private> u_xlatb55 : bool;

@group(0u) @binding(2u) var unity_LightmapInd : texture_2d<f32>;

@group(0u) @binding(10u) var samplerunity_Lightmap : sampler;

@group(0u) @binding(1u) var unity_Lightmap : texture_2d<f32>;

var<private> u_xlat83 : f32;

var<private> u_xlat84 : f32;

var<private> u_xlat33 : f32;

var<private> u_xlatb59 : bool;

var<private> u_xlat59 : vec2<f32>;

var<private> u_xlat60 : vec2<f32>;

var<private> u_xlat85 : f32;

var<private> u_xlat66 : vec2<f32>;

var<private> u_xlat87 : f32;

var<private> u_xlat28 : vec3<f32>;

var<private> u_xlat54 : f32;

var<private> u_xlatb28 : vec2<bool>;

struct tint_padded_array_element {
  @size(16u)
  tint_element : f32,
}

struct LightCookies_1_1 {
  m_24 : mat4x4<f32>,
  m_25 : f32,
  m_26 : f32,
  m_27 : f32,
  m_28 : array<mat4x4<f32>, 32u>,
  m_29 : array<vec4<f32>, 32u>,
  m_30 : array<tint_padded_array_element, 32u>,
}

@group(1u) @binding(4u) var<uniform> v_5 : LightCookies_1_1;

@group(0u) @binding(5u) var v_6 : texture_2d<f32>;

@group(0u) @binding(13u) var sampler_MainLightCookieTexture : sampler;

@group(0u) @binding(0u) var unity_SpecCube0 : texture_cube<f32>;

@group(0u) @binding(9u) var samplerunity_SpecCube0 : sampler;

var<private> u_xlatu84 : u32;

var<private> u_xlati85 : i32;

var<private> u_xlati84 : i32;

struct AdditionalLights_1 {
  m_31 : array<vec4<f32>, 32u>,
  m_32 : array<vec4<f32>, 32u>,
  m_33 : array<vec4<f32>, 32u>,
  m_34 : array<vec4<f32>, 32u>,
  m_35 : array<vec4<f32>, 32u>,
  m_36 : array<tint_padded_array_element, 32u>,
}

@group(1u) @binding(1u) var<uniform> v_7 : AdditionalLights_1;

var<private> u_xlat86 : f32;

var<private> u_xlati87 : i32;

var<private> u_xlatb88 : bool;

var<private> u_xlatb11 : vec4<bool>;

var<private> u_xlat89 : f32;

var<private> u_xlat37 : vec3<f32>;

var<private> u_xlat88 : f32;

var<private> u_xlatb87 : bool;

@group(0u) @binding(4u) var v_8 : texture_depth_2d;

var<private> u_xlat64 : vec2<f32>;

var<private> u_xlat39 : vec3<f32>;

var<private> u_xlat22 : vec4<f32>;

var<private> u_xlat40 : vec2<f32>;

var<private> u_xlat72 : vec2<f32>;

var<private> u_xlat67 : vec2<f32>;

var<private> u_xlat23 : vec4<f32>;

var<private> u_xlat24 : vec4<f32>;

var<private> u_xlat25 : vec4<f32>;

var<private> u_xlati88 : i32;

var<private> u_xlati11 : i32;

var<private> u_xlati37 : i32;

var<private> u_xlatb37 : vec3<bool>;

@group(0u) @binding(6u) var v_9 : texture_2d<f32>;

@group(0u) @binding(11u) var sampler_LinearClamp : sampler;

var<private> u_xlat78 : f32;

var<private> SV_Target0 : vec4<f32>;

fn main_inner(vs_INTERP9 : vec3<f32>, vs_INTERP4 : vec4<f32>, vs_INTERP8 : vec3<f32>, vs_INTERP5 : vec4<f32>, vs_INTERP6 : vec4<f32>, vs_INTERP0 : vec2<f32>, gl_FragCoord : vec4<f32>) {
  var v_10 : vec3<f32>;
  var txVec0 : vec3<f32>;
  var txVec1 : vec3<f32>;
  var txVec2 : vec3<f32>;
  var txVec3 : vec3<f32>;
  var txVec4 : vec3<f32>;
  var txVec5 : vec3<f32>;
  var txVec6 : vec3<f32>;
  var txVec7 : vec3<f32>;
  var txVec8 : vec3<f32>;
  var txVec9 : vec3<f32>;
  var txVec10 : vec3<f32>;
  var txVec11 : vec3<f32>;
  var txVec12 : vec3<f32>;
  var txVec13 : vec3<f32>;
  var txVec14 : vec3<f32>;
  var txVec15 : vec3<f32>;
  var txVec16 : vec3<f32>;
  var txVec17 : vec3<f32>;
  var txVec18 : vec3<f32>;
  var txVec19 : vec3<f32>;
  var txVec20 : vec3<f32>;
  var txVec21 : vec3<f32>;
  var txVec22 : vec3<f32>;
  var txVec23 : vec3<f32>;
  var txVec24 : vec3<f32>;
  var txVec25 : vec3<f32>;
  var txVec26 : vec3<f32>;
  var txVec27 : vec3<f32>;
  var txVec28 : vec3<f32>;
  var txVec29 : vec3<f32>;
  var v_11 : f32;
  var param : i32;
  var param_1 : i32;
  var param_2 : i32;
  var param_3 : i32;
  var v_12 : f32;
  var v_13 : f32;
  var txVec30 : vec3<f32>;
  var txVec31 : vec3<f32>;
  var txVec32 : vec3<f32>;
  var txVec33 : vec3<f32>;
  var txVec34 : vec3<f32>;
  var txVec35 : vec3<f32>;
  var txVec36 : vec3<f32>;
  var txVec37 : vec3<f32>;
  var txVec38 : vec3<f32>;
  var txVec39 : vec3<f32>;
  var txVec40 : vec3<f32>;
  var txVec41 : vec3<f32>;
  var txVec42 : vec3<f32>;
  var txVec43 : vec3<f32>;
  var txVec44 : vec3<f32>;
  var txVec45 : vec3<f32>;
  var txVec46 : vec3<f32>;
  var txVec47 : vec3<f32>;
  var txVec48 : vec3<f32>;
  var txVec49 : vec3<f32>;
  var txVec50 : vec3<f32>;
  var txVec51 : vec3<f32>;
  var txVec52 : vec3<f32>;
  var txVec53 : vec3<f32>;
  var txVec54 : vec3<f32>;
  var txVec55 : vec3<f32>;
  var txVec56 : vec3<f32>;
  var txVec57 : vec3<f32>;
  var txVec58 : vec3<f32>;
  var txVec59 : vec3<f32>;
  var v_14 : f32;
  var v_15 : f32;
  var v_16 : vec3<f32>;
  var u_xlatu_loop_1 : u32;
  var indexable : array<vec4<u32>, 4u>;
  var v_17 : f32;
  var v_18 : f32;
  var txVec60 : vec3<f32>;
  var txVec61 : vec3<f32>;
  var txVec62 : vec3<f32>;
  var txVec63 : vec3<f32>;
  var txVec64 : vec3<f32>;
  var txVec65 : vec3<f32>;
  var txVec66 : vec3<f32>;
  var txVec67 : vec3<f32>;
  var txVec68 : vec3<f32>;
  var txVec69 : vec3<f32>;
  var txVec70 : vec3<f32>;
  var txVec71 : vec3<f32>;
  var txVec72 : vec3<f32>;
  var txVec73 : vec3<f32>;
  var txVec74 : vec3<f32>;
  var txVec75 : vec3<f32>;
  var txVec76 : vec3<f32>;
  var txVec77 : vec3<f32>;
  var txVec78 : vec3<f32>;
  var txVec79 : vec3<f32>;
  var txVec80 : vec3<f32>;
  var txVec81 : vec3<f32>;
  var txVec82 : vec3<f32>;
  var txVec83 : vec3<f32>;
  var txVec84 : vec3<f32>;
  var txVec85 : vec3<f32>;
  var txVec86 : vec3<f32>;
  var txVec87 : vec3<f32>;
  var txVec88 : vec3<f32>;
  var txVec89 : vec3<f32>;
  var v_19 : f32;
  var v_20 : f32;
  var v_21 : f32;
  var v_22 : vec3<f32>;
  var u_xlat_precise_vec4 : vec4<f32>;
  var u_xlat_precise_ivec4 : vec4<i32>;
  var u_xlat_precise_bvec4 : vec4<bool>;
  var u_xlat_precise_uvec4 : vec4<u32>;
  u_xlat0.x = dot(vs_INTERP9, vs_INTERP9);
  u_xlat0.x = sqrt(u_xlat0.x);
  u_xlat0.x = (1.0f / u_xlat0.x);
  u_xlatb26.x = (0.0f < vs_INTERP4.w);
  u_xlatb26.y = (v.unity_WorldTransformParams.w >= 0.0f);
  u_xlat26.x = select(-1.0f, 1.0f, u_xlatb26.x);
  u_xlat26.y = select(-1.0f, 1.0f, u_xlatb26.y);
  u_xlat26.x = (u_xlat26.y * u_xlat26.x);
  u_xlat1 = (vs_INTERP4.yzx * vs_INTERP9.zxy);
  u_xlat1 = ((vs_INTERP9.yzx * vs_INTERP4.zxy) + -(u_xlat1));
  u_xlat26 = (u_xlat26.xxx * u_xlat1);
  u_xlat1 = (u_xlat0.xxx * vs_INTERP9);
  let v_23 = (u_xlat0.xxx * vs_INTERP4.xyz);
  u_xlat2 = vec4<f32>(v_23.xyz, u_xlat2.w);
  let v_24 = (u_xlat26 * u_xlat0.xxx);
  u_xlat3 = vec4<f32>(v_24.xyz, u_xlat3.w);
  u_xlatb0 = (v_1.unity_OrthoParams.w == 0.0f);
  u_xlat4 = (-(vs_INTERP8) + v_1.m_5);
  u_xlat79 = dot(u_xlat4, u_xlat4);
  u_xlat79 = inverseSqrt(u_xlat79);
  let v_25 = u_xlat79;
  u_xlat4 = (vec3<f32>(v_25, v_25, v_25) * u_xlat4);
  u_xlat5.x = v_1.unity_MatrixV[0i].z;
  u_xlat5.y = v_1.unity_MatrixV[1i].z;
  u_xlat5.z = v_1.unity_MatrixV[2i].z;
  if (u_xlatb0) {
    v_10 = u_xlat4;
  } else {
    v_10 = u_xlat5.xyz;
  }
  u_xlat4 = v_10;
  let v_26 = (u_xlat4.yyy * v.unity_WorldToObject[1i].xyz);
  u_xlat5 = vec4<f32>(v_26.xyz, u_xlat5.w);
  let v_27 = ((v.unity_WorldToObject[0i].xyz * u_xlat4.xxx) + u_xlat5.xyz);
  u_xlat5 = vec4<f32>(v_27.xyz, u_xlat5.w);
  let v_28 = ((v.unity_WorldToObject[2i].xyz * u_xlat4.zzz) + u_xlat5.xyz);
  u_xlat5 = vec4<f32>(v_28.xyz, u_xlat5.w);
  u_xlat0.x = dot(u_xlat5.xyz, u_xlat5.xyz);
  u_xlat0.x = inverseSqrt(u_xlat0.x);
  let v_29 = (u_xlat0.xxx * u_xlat5.xyz);
  u_xlat5 = vec4<f32>(v_29.xyz, u_xlat5.w);
  u_xlat6 = textureSampleBias(Texture2D_B222E8F, samplerTexture2D_B222E8F, vs_INTERP5.xy, v_1.m.x);
  u_xlat7 = (u_xlat6.xyz * v_2.Color_C30C7CA3.xyz);
  u_xlat8 = textureSampleBias(Texture2D_D9BFD5F1, samplerTexture2D_D9BFD5F1, vs_INTERP5.xy, v_1.m.x).wxyz;
  u_xlat9 = ((u_xlat8.yzwx * vec4<f32>(2.0f)) + vec4<f32>(-1.0f));
  u_xlat0.x = dot(u_xlat9, u_xlat9);
  u_xlat0.x = inverseSqrt(u_xlat0.x);
  u_xlat34 = (u_xlat0.xxx * u_xlat9.xyz);
  u_xlat0.x = (vs_INTERP6.y * 200.0f);
  u_xlat0.x = min(u_xlat0.x, 1.0f);
  let v_30 = (u_xlat0.xxx * u_xlat6.xyz);
  u_xlat6 = vec4<f32>(v_30.xyz, u_xlat6.w);
  let v_31 = (u_xlat3.xyz * u_xlat34.yyy);
  u_xlat3 = vec4<f32>(v_31.xyz, u_xlat3.w);
  let v_32 = ((u_xlat34.xxx * u_xlat2.xyz) + u_xlat3.xyz);
  u_xlat2 = vec4<f32>(v_32.xyz, u_xlat2.w);
  u_xlat1 = ((u_xlat34.zzz * u_xlat1) + u_xlat2.xyz);
  u_xlat0.x = dot(u_xlat1, u_xlat1);
  u_xlat0.x = max(u_xlat0.x, 1.17549435e-38f);
  u_xlat0.x = inverseSqrt(u_xlat0.x);
  u_xlat1 = (u_xlat0.xxx * u_xlat1);
  let v_33 = (vs_INTERP8 + -(v_3.m_9.xyz));
  u_xlat2 = vec4<f32>(v_33.xyz, u_xlat2.w);
  let v_34 = (vs_INTERP8 + -(v_3.m_10.xyz));
  u_xlat3 = vec4<f32>(v_34.xyz, u_xlat3.w);
  let v_35 = (vs_INTERP8 + -(v_3.m_11.xyz));
  u_xlat9 = vec4<f32>(v_35.xyz, u_xlat9.w);
  let v_36 = (vs_INTERP8 + -(v_3.m_12.xyz));
  u_xlat10 = vec4<f32>(v_36.xyz, u_xlat10.w);
  u_xlat2.x = dot(u_xlat2.xyz, u_xlat2.xyz);
  u_xlat2.y = dot(u_xlat3.xyz, u_xlat3.xyz);
  u_xlat2.z = dot(u_xlat9.xyz, u_xlat9.xyz);
  u_xlat2.w = dot(u_xlat10.xyz, u_xlat10.xyz);
  u_xlatb2 = (u_xlat2 < v_3.m_13);
  u_xlat3.x = select(0.0f, 1.0f, u_xlatb2.x);
  u_xlat3.y = select(0.0f, 1.0f, u_xlatb2.y);
  u_xlat3.z = select(0.0f, 1.0f, u_xlatb2.z);
  u_xlat3.w = select(0.0f, 1.0f, u_xlatb2.w);
  u_xlat2.x = select(0.0f, -1.0f, u_xlatb2.x);
  u_xlat2.y = select(0.0f, -1.0f, u_xlatb2.y);
  u_xlat2.z = select(0.0f, -1.0f, u_xlatb2.z);
  let v_37 = (u_xlat2.xyz + u_xlat3.yzw);
  u_xlat2 = vec4<f32>(v_37.xyz, u_xlat2.w);
  let v_38 = max(u_xlat2.xyz, vec3<f32>());
  u_xlat3 = vec4<f32>(u_xlat3.x, v_38.xyz);
  u_xlat0.x = dot(u_xlat3, vec4<f32>(4.0f, 3.0f, 2.0f, 1.0f));
  u_xlat0.x = (-(u_xlat0.x) + 4.0f);
  u_xlatu0 = u32(u_xlat0.x);
  u_xlati0 = (bitcast<i32>(u_xlatu0) << bitcast<u32>(2i));
  let v_39 = (vs_INTERP8.yyy * v_3.m_8[((u_xlati0 + 1i) / 4i)][((u_xlati0 + 1i) % 4i)].xyz);
  u_xlat2 = vec4<f32>(v_39.xyz, u_xlat2.w);
  let v_40 = ((v_3.m_8[(u_xlati0 / 4i)][(u_xlati0 % 4i)].xyz * vs_INTERP8.xxx) + u_xlat2.xyz);
  u_xlat2 = vec4<f32>(v_40.xyz, u_xlat2.w);
  let v_41 = ((v_3.m_8[((u_xlati0 + 2i) / 4i)][((u_xlati0 + 2i) % 4i)].xyz * vs_INTERP8.zzz) + u_xlat2.xyz);
  u_xlat2 = vec4<f32>(v_41.xyz, u_xlat2.w);
  let v_42 = (u_xlat2.xyz + v_3.m_8[((u_xlati0 + 3i) / 4i)][((u_xlati0 + 3i) % 4i)].xyz);
  u_xlat2 = vec4<f32>(v_42.xyz, u_xlat2.w);
  u_xlatb0 = (0.0f < v_3.m_16.y);
  if (u_xlatb0) {
    u_xlatb79 = (v_3.m_16.y == 1.0f);
    if (u_xlatb79) {
      u_xlat3 = (u_xlat2.xyxy + v_3.m_14);
      let v_43 = u_xlat3.xy;
      txVec0 = vec3<f32>(v_43.x, v_43.y, u_xlat2.z);
      let v_44 = txVec0;
      u_xlat9.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_44.xy, v_44.z);
      let v_45 = u_xlat3.zw;
      txVec1 = vec3<f32>(v_45.x, v_45.y, u_xlat2.z);
      let v_46 = txVec1;
      u_xlat9.y = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_46.xy, v_46.z);
      u_xlat3 = (u_xlat2.xyxy + v_3.m_15);
      let v_47 = u_xlat3.xy;
      txVec2 = vec3<f32>(v_47.x, v_47.y, u_xlat2.z);
      let v_48 = txVec2;
      u_xlat9.z = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_48.xy, v_48.z);
      let v_49 = u_xlat3.zw;
      txVec3 = vec3<f32>(v_49.x, v_49.y, u_xlat2.z);
      let v_50 = txVec3;
      u_xlat9.w = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_50.xy, v_50.z);
      u_xlat79 = dot(u_xlat9, vec4<f32>(0.25f));
    } else {
      u_xlatb80 = (v_3.m_16.y == 2.0f);
      if (u_xlatb80) {
        let v_51 = ((u_xlat2.xy * v_3.m_17.zw) + vec2<f32>(0.5f));
        u_xlat3 = vec4<f32>(v_51.xy, u_xlat3.zw);
        let v_52 = floor(u_xlat3.xy);
        u_xlat3 = vec4<f32>(v_52.xy, u_xlat3.zw);
        u_xlat55 = ((u_xlat2.xy * v_3.m_17.zw) + -(u_xlat3.xy));
        u_xlat9 = (u_xlat55.xxyy + vec4<f32>(0.5f, 1.0f, 0.5f, 1.0f));
        u_xlat10 = (u_xlat9.xxzz * u_xlat9.xxzz);
        let v_53 = (u_xlat10.yw * vec2<f32>(0.07999999821186065674f));
        let v_54 = u_xlat9;
        u_xlat9 = vec4<f32>(v_53.x, v_54.y, v_53.y, v_54.w);
        let v_55 = ((u_xlat10.xz * vec2<f32>(0.5f)) + -(u_xlat55));
        u_xlat10 = vec4<f32>(v_55.xy, u_xlat10.zw);
        u_xlat62 = (-(u_xlat55) + vec2<f32>(1.0f));
        let v_56 = min(u_xlat55, vec2<f32>());
        u_xlat11 = vec4<f32>(v_56.xy, u_xlat11.zw);
        let v_57 = ((-(u_xlat11.xy) * u_xlat11.xy) + u_xlat62);
        u_xlat11 = vec4<f32>(v_57.xy, u_xlat11.zw);
        u_xlat55 = max(u_xlat55, vec2<f32>());
        u_xlat55 = ((-(u_xlat55) * u_xlat55) + u_xlat9.yw);
        let v_58 = (u_xlat11.xy + vec2<f32>(1.0f));
        u_xlat11 = vec4<f32>(v_58.xy, u_xlat11.zw);
        u_xlat55 = (u_xlat55 + vec2<f32>(1.0f));
        let v_59 = (u_xlat10.xy * vec2<f32>(0.15999999642372131348f));
        u_xlat12 = vec4<f32>(v_59.xy, u_xlat12.zw);
        let v_60 = (u_xlat62 * vec2<f32>(0.15999999642372131348f));
        u_xlat10 = vec4<f32>(v_60.xy, u_xlat10.zw);
        let v_61 = (u_xlat11.xy * vec2<f32>(0.15999999642372131348f));
        u_xlat11 = vec4<f32>(v_61.xy, u_xlat11.zw);
        let v_62 = (u_xlat55 * vec2<f32>(0.15999999642372131348f));
        u_xlat13 = vec4<f32>(v_62.xy, u_xlat13.zw);
        u_xlat55 = (u_xlat9.yw * vec2<f32>(0.15999999642372131348f));
        u_xlat12.z = u_xlat11.x;
        u_xlat12.w = u_xlat55.x;
        u_xlat10.z = u_xlat13.x;
        u_xlat10.w = u_xlat9.x;
        u_xlat14 = (u_xlat10.zwxz + u_xlat12.zwxz);
        u_xlat11.z = u_xlat12.y;
        u_xlat11.w = u_xlat55.y;
        u_xlat13.z = u_xlat10.y;
        u_xlat13.w = u_xlat9.z;
        let v_63 = (u_xlat11.zyw + u_xlat13.zyw);
        u_xlat9 = vec4<f32>(v_63.xyz, u_xlat9.w);
        let v_64 = (u_xlat10.xzw / u_xlat14.zwy);
        u_xlat10 = vec4<f32>(v_64.xyz, u_xlat10.w);
        let v_65 = (u_xlat10.xyz + vec3<f32>(-2.5f, -0.5f, 1.5f));
        u_xlat10 = vec4<f32>(v_65.xyz, u_xlat10.w);
        let v_66 = (u_xlat13.zyw / u_xlat9.xyz);
        u_xlat11 = vec4<f32>(v_66.xyz, u_xlat11.w);
        let v_67 = (u_xlat11.xyz + vec3<f32>(-2.5f, -0.5f, 1.5f));
        u_xlat11 = vec4<f32>(v_67.xyz, u_xlat11.w);
        let v_68 = (u_xlat10.yxz * v_3.m_17.xxx);
        u_xlat10 = vec4<f32>(v_68.xyz, u_xlat10.w);
        let v_69 = (u_xlat11.xyz * v_3.m_17.yyy);
        u_xlat11 = vec4<f32>(v_69.xyz, u_xlat11.w);
        u_xlat10.w = u_xlat11.x;
        u_xlat12 = ((u_xlat3.xyxy * v_3.m_17.xyxy) + u_xlat10.ywxw);
        u_xlat55 = ((u_xlat3.xy * v_3.m_17.xy) + u_xlat10.zw);
        u_xlat11.w = u_xlat10.y;
        let v_70 = u_xlat11.yz;
        let v_71 = u_xlat10;
        u_xlat10 = vec4<f32>(v_71.x, v_70.x, v_71.z, v_70.y);
        u_xlat13 = ((u_xlat3.xyxy * v_3.m_17.xyxy) + u_xlat10.xyzy);
        u_xlat11 = ((u_xlat3.xyxy * v_3.m_17.xyxy) + u_xlat11.wywz);
        u_xlat10 = ((u_xlat3.xyxy * v_3.m_17.xyxy) + u_xlat10.xwzw);
        u_xlat15 = (u_xlat9.xxxy * u_xlat14.zwyz);
        u_xlat16 = (u_xlat9.yyzz * u_xlat14);
        u_xlat80 = (u_xlat9.z * u_xlat14.y);
        let v_72 = u_xlat12.xy;
        txVec4 = vec3<f32>(v_72.x, v_72.y, u_xlat2.z);
        let v_73 = txVec4;
        u_xlat3.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_73.xy, v_73.z);
        let v_74 = u_xlat12.zw;
        txVec5 = vec3<f32>(v_74.x, v_74.y, u_xlat2.z);
        let v_75 = txVec5;
        u_xlat29 = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_75.xy, v_75.z);
        u_xlat29 = (u_xlat29 * u_xlat15.y);
        u_xlat3.x = ((u_xlat15.x * u_xlat3.x) + u_xlat29);
        let v_76 = u_xlat55;
        txVec6 = vec3<f32>(v_76.x, v_76.y, u_xlat2.z);
        let v_77 = txVec6;
        u_xlat29 = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_77.xy, v_77.z);
        u_xlat3.x = ((u_xlat15.z * u_xlat29) + u_xlat3.x);
        let v_78 = u_xlat11.xy;
        txVec7 = vec3<f32>(v_78.x, v_78.y, u_xlat2.z);
        let v_79 = txVec7;
        u_xlat29 = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_79.xy, v_79.z);
        u_xlat3.x = ((u_xlat15.w * u_xlat29) + u_xlat3.x);
        let v_80 = u_xlat13.xy;
        txVec8 = vec3<f32>(v_80.x, v_80.y, u_xlat2.z);
        let v_81 = txVec8;
        u_xlat29 = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_81.xy, v_81.z);
        u_xlat3.x = ((u_xlat16.x * u_xlat29) + u_xlat3.x);
        let v_82 = u_xlat13.zw;
        txVec9 = vec3<f32>(v_82.x, v_82.y, u_xlat2.z);
        let v_83 = txVec9;
        u_xlat29 = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_83.xy, v_83.z);
        u_xlat3.x = ((u_xlat16.y * u_xlat29) + u_xlat3.x);
        let v_84 = u_xlat11.zw;
        txVec10 = vec3<f32>(v_84.x, v_84.y, u_xlat2.z);
        let v_85 = txVec10;
        u_xlat29 = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_85.xy, v_85.z);
        u_xlat3.x = ((u_xlat16.z * u_xlat29) + u_xlat3.x);
        let v_86 = u_xlat10.xy;
        txVec11 = vec3<f32>(v_86.x, v_86.y, u_xlat2.z);
        let v_87 = txVec11;
        u_xlat29 = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_87.xy, v_87.z);
        u_xlat3.x = ((u_xlat16.w * u_xlat29) + u_xlat3.x);
        let v_88 = u_xlat10.zw;
        txVec12 = vec3<f32>(v_88.x, v_88.y, u_xlat2.z);
        let v_89 = txVec12;
        u_xlat29 = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_89.xy, v_89.z);
        u_xlat79 = ((u_xlat80 * u_xlat29) + u_xlat3.x);
      } else {
        let v_90 = ((u_xlat2.xy * v_3.m_17.zw) + vec2<f32>(0.5f));
        u_xlat3 = vec4<f32>(v_90.xy, u_xlat3.zw);
        let v_91 = floor(u_xlat3.xy);
        u_xlat3 = vec4<f32>(v_91.xy, u_xlat3.zw);
        u_xlat55 = ((u_xlat2.xy * v_3.m_17.zw) + -(u_xlat3.xy));
        u_xlat9 = (u_xlat55.xxyy + vec4<f32>(0.5f, 1.0f, 0.5f, 1.0f));
        u_xlat10 = (u_xlat9.xxzz * u_xlat9.xxzz);
        let v_92 = (u_xlat10.yw * vec2<f32>(0.04081600159406661987f));
        let v_93 = u_xlat11;
        u_xlat11 = vec4<f32>(v_93.x, v_92.x, v_93.z, v_92.y);
        let v_94 = ((u_xlat10.xz * vec2<f32>(0.5f)) + -(u_xlat55));
        let v_95 = u_xlat9;
        u_xlat9 = vec4<f32>(v_94.x, v_95.y, v_94.y, v_95.w);
        let v_96 = (-(u_xlat55) + vec2<f32>(1.0f));
        u_xlat10 = vec4<f32>(v_96.xy, u_xlat10.zw);
        u_xlat62 = min(u_xlat55, vec2<f32>());
        let v_97 = ((-(u_xlat62) * u_xlat62) + u_xlat10.xy);
        u_xlat10 = vec4<f32>(v_97.xy, u_xlat10.zw);
        u_xlat62 = max(u_xlat55, vec2<f32>());
        let v_98 = ((-(u_xlat62) * u_xlat62) + u_xlat9.yw);
        u_xlat35 = vec3<f32>(v_98.x, u_xlat35.y, v_98.y);
        let v_99 = (u_xlat10.xy + vec2<f32>(2.0f));
        u_xlat10 = vec4<f32>(v_99.xy, u_xlat10.zw);
        let v_100 = (u_xlat35.xz + vec2<f32>(2.0f));
        let v_101 = u_xlat9;
        u_xlat9 = vec4<f32>(v_101.x, v_100.x, v_101.z, v_100.y);
        u_xlat12.z = (u_xlat9.y * 0.08163200318813323975f);
        let v_102 = (u_xlat9.zxw * vec3<f32>(0.08163200318813323975f));
        u_xlat13 = vec4<f32>(v_102.xyz, u_xlat13.w);
        let v_103 = (u_xlat10.xy * vec2<f32>(0.08163200318813323975f));
        u_xlat9 = vec4<f32>(v_103.xy, u_xlat9.zw);
        u_xlat12.x = u_xlat13.y;
        let v_104 = ((u_xlat55.xx * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.16326400637626647949f, 0.08163200318813323975f));
        let v_105 = u_xlat12;
        u_xlat12 = vec4<f32>(v_105.x, v_104.x, v_105.z, v_104.y);
        let v_106 = ((u_xlat55.xx * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.08163200318813323975f, 0.16326400637626647949f));
        let v_107 = u_xlat10;
        u_xlat10 = vec4<f32>(v_106.x, v_107.y, v_106.y, v_107.w);
        u_xlat10.y = u_xlat9.x;
        u_xlat10.w = u_xlat11.y;
        u_xlat12 = (u_xlat10 + u_xlat12);
        let v_108 = ((u_xlat55.yy * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.16326400637626647949f, 0.08163200318813323975f));
        let v_109 = u_xlat13;
        u_xlat13 = vec4<f32>(v_109.x, v_108.x, v_109.z, v_108.y);
        let v_110 = ((u_xlat55.yy * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.08163200318813323975f, 0.16326400637626647949f));
        let v_111 = u_xlat11;
        u_xlat11 = vec4<f32>(v_110.x, v_111.y, v_110.y, v_111.w);
        u_xlat11.y = u_xlat9.y;
        u_xlat9 = (u_xlat11 + u_xlat13);
        u_xlat10 = (u_xlat10 / u_xlat12);
        u_xlat10 = (u_xlat10 + vec4<f32>(-3.5f, -1.5f, 0.5f, 2.5f));
        u_xlat11 = (u_xlat11 / u_xlat9);
        u_xlat11 = (u_xlat11 + vec4<f32>(-3.5f, -1.5f, 0.5f, 2.5f));
        u_xlat10 = (u_xlat10.wxyz * v_3.m_17.xxxx);
        u_xlat11 = (u_xlat11.xwyz * v_3.m_17.yyyy);
        let v_112 = u_xlat10.yzw;
        u_xlat13 = vec4<f32>(v_112.x, u_xlat13.y, v_112.yz);
        u_xlat13.y = u_xlat11.x;
        u_xlat14 = ((u_xlat3.xyxy * v_3.m_17.xyxy) + u_xlat13.xyzy);
        u_xlat55 = ((u_xlat3.xy * v_3.m_17.xy) + u_xlat13.wy);
        u_xlat10.y = u_xlat13.y;
        u_xlat13.y = u_xlat11.z;
        u_xlat15 = ((u_xlat3.xyxy * v_3.m_17.xyxy) + u_xlat13.xyzy);
        let v_113 = ((u_xlat3.xy * v_3.m_17.xy) + u_xlat13.wy);
        u_xlat16 = vec4<f32>(v_113.xy, u_xlat16.zw);
        u_xlat10.z = u_xlat13.y;
        u_xlat17 = ((u_xlat3.xyxy * v_3.m_17.xyxy) + u_xlat10.xyxz);
        u_xlat13.y = u_xlat11.w;
        u_xlat18 = ((u_xlat3.xyxy * v_3.m_17.xyxy) + u_xlat13.xyzy);
        u_xlat36 = ((u_xlat3.xy * v_3.m_17.xy) + u_xlat13.wy);
        u_xlat10.w = u_xlat13.y;
        u_xlat68 = ((u_xlat3.xy * v_3.m_17.xy) + u_xlat10.xw);
        let v_114 = u_xlat13.xzw;
        u_xlat11 = vec4<f32>(v_114.x, u_xlat11.y, v_114.yz);
        u_xlat13 = ((u_xlat3.xyxy * v_3.m_17.xyxy) + u_xlat11.xyzy);
        u_xlat63 = ((u_xlat3.xy * v_3.m_17.xy) + u_xlat11.wy);
        u_xlat11.x = u_xlat10.x;
        let v_115 = ((u_xlat3.xy * v_3.m_17.xy) + u_xlat11.xy);
        u_xlat3 = vec4<f32>(v_115.xy, u_xlat3.zw);
        u_xlat19 = (u_xlat9.xxxx * u_xlat12);
        u_xlat20 = (u_xlat9.yyyy * u_xlat12);
        u_xlat21 = (u_xlat9.zzzz * u_xlat12);
        u_xlat9 = (u_xlat9.wwww * u_xlat12);
        let v_116 = u_xlat14.xy;
        txVec13 = vec3<f32>(v_116.x, v_116.y, u_xlat2.z);
        let v_117 = txVec13;
        u_xlat80 = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_117.xy, v_117.z);
        let v_118 = u_xlat14.zw;
        txVec14 = vec3<f32>(v_118.x, v_118.y, u_xlat2.z);
        let v_119 = txVec14;
        u_xlat82 = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_119.xy, v_119.z);
        u_xlat82 = (u_xlat82 * u_xlat19.y);
        u_xlat80 = ((u_xlat19.x * u_xlat80) + u_xlat82);
        let v_120 = u_xlat55;
        txVec15 = vec3<f32>(v_120.x, v_120.y, u_xlat2.z);
        let v_121 = txVec15;
        u_xlat55.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_121.xy, v_121.z);
        u_xlat80 = ((u_xlat19.z * u_xlat55.x) + u_xlat80);
        let v_122 = u_xlat17.xy;
        txVec16 = vec3<f32>(v_122.x, v_122.y, u_xlat2.z);
        let v_123 = txVec16;
        u_xlat55.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_123.xy, v_123.z);
        u_xlat80 = ((u_xlat19.w * u_xlat55.x) + u_xlat80);
        let v_124 = u_xlat15.xy;
        txVec17 = vec3<f32>(v_124.x, v_124.y, u_xlat2.z);
        let v_125 = txVec17;
        u_xlat55.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_125.xy, v_125.z);
        u_xlat80 = ((u_xlat20.x * u_xlat55.x) + u_xlat80);
        let v_126 = u_xlat15.zw;
        txVec18 = vec3<f32>(v_126.x, v_126.y, u_xlat2.z);
        let v_127 = txVec18;
        u_xlat55.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_127.xy, v_127.z);
        u_xlat80 = ((u_xlat20.y * u_xlat55.x) + u_xlat80);
        let v_128 = u_xlat16.xy;
        txVec19 = vec3<f32>(v_128.x, v_128.y, u_xlat2.z);
        let v_129 = txVec19;
        u_xlat55.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_129.xy, v_129.z);
        u_xlat80 = ((u_xlat20.z * u_xlat55.x) + u_xlat80);
        let v_130 = u_xlat17.zw;
        txVec20 = vec3<f32>(v_130.x, v_130.y, u_xlat2.z);
        let v_131 = txVec20;
        u_xlat55.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_131.xy, v_131.z);
        u_xlat80 = ((u_xlat20.w * u_xlat55.x) + u_xlat80);
        let v_132 = u_xlat18.xy;
        txVec21 = vec3<f32>(v_132.x, v_132.y, u_xlat2.z);
        let v_133 = txVec21;
        u_xlat55.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_133.xy, v_133.z);
        u_xlat80 = ((u_xlat21.x * u_xlat55.x) + u_xlat80);
        let v_134 = u_xlat18.zw;
        txVec22 = vec3<f32>(v_134.x, v_134.y, u_xlat2.z);
        let v_135 = txVec22;
        u_xlat55.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_135.xy, v_135.z);
        u_xlat80 = ((u_xlat21.y * u_xlat55.x) + u_xlat80);
        let v_136 = u_xlat36;
        txVec23 = vec3<f32>(v_136.x, v_136.y, u_xlat2.z);
        let v_137 = txVec23;
        u_xlat55.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_137.xy, v_137.z);
        u_xlat80 = ((u_xlat21.z * u_xlat55.x) + u_xlat80);
        let v_138 = u_xlat68;
        txVec24 = vec3<f32>(v_138.x, v_138.y, u_xlat2.z);
        let v_139 = txVec24;
        u_xlat55.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_139.xy, v_139.z);
        u_xlat80 = ((u_xlat21.w * u_xlat55.x) + u_xlat80);
        let v_140 = u_xlat13.xy;
        txVec25 = vec3<f32>(v_140.x, v_140.y, u_xlat2.z);
        let v_141 = txVec25;
        u_xlat55.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_141.xy, v_141.z);
        u_xlat80 = ((u_xlat9.x * u_xlat55.x) + u_xlat80);
        let v_142 = u_xlat13.zw;
        txVec26 = vec3<f32>(v_142.x, v_142.y, u_xlat2.z);
        let v_143 = txVec26;
        u_xlat55.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_143.xy, v_143.z);
        u_xlat80 = ((u_xlat9.y * u_xlat55.x) + u_xlat80);
        let v_144 = u_xlat63;
        txVec27 = vec3<f32>(v_144.x, v_144.y, u_xlat2.z);
        let v_145 = txVec27;
        u_xlat55.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_145.xy, v_145.z);
        u_xlat80 = ((u_xlat9.z * u_xlat55.x) + u_xlat80);
        let v_146 = u_xlat3.xy;
        txVec28 = vec3<f32>(v_146.x, v_146.y, u_xlat2.z);
        let v_147 = txVec28;
        u_xlat3.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_147.xy, v_147.z);
        u_xlat79 = ((u_xlat9.w * u_xlat3.x) + u_xlat80);
      }
    }
  } else {
    let v_148 = u_xlat2.xy;
    txVec29 = vec3<f32>(v_148.x, v_148.y, u_xlat2.z);
    let v_149 = txVec29;
    u_xlat79 = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_149.xy, v_149.z);
  }
  u_xlat80 = (-(v_3.m_16.x) + 1.0f);
  u_xlat79 = ((u_xlat79 * v_3.m_16.x) + u_xlat80);
  u_xlatb3.x = (0.0f >= u_xlat2.z);
  u_xlatb29 = (u_xlat2.z >= 1.0f);
  u_xlatb3.x = (u_xlatb29 | u_xlatb3.x);
  let v_150 = u_xlatb3.x;
  u_xlat79 = select(u_xlat79, 1.0f, v_150);
  u_xlat1.x = dot(u_xlat1, -(v_1.m_2.xyz));
  u_xlat1.x = clamp(u_xlat1.x, 0.0f, 1.0f);
  let v_151 = u_xlat79;
  u_xlat27 = (vec3<f32>(v_151, v_151, v_151) * v_1.m_3.xyz);
  u_xlat1 = (u_xlat27 * u_xlat1.xxx);
  u_xlat1 = (u_xlat1 * u_xlat6.xyz);
  u_xlatb79 = (v.unity_LODFade.x < 0.0f);
  u_xlat29 = (v.unity_LODFade.x + 1.0f);
  if (u_xlatb79) {
    v_11 = u_xlat29;
  } else {
    v_11 = v.unity_LODFade.x;
  }
  u_xlat79 = v_11;
  u_xlatb29 = (0.5f >= u_xlat79);
  let v_152 = (abs(u_xlat5.xyz) * v_1.m_7.xyx);
  u_xlat5 = vec4<f32>(v_152.xyz, u_xlat5.w);
  u_xlatu5 = vec3<u32>(u_xlat5.xyz);
  u_xlatu55 = (u_xlatu5.z * 1025u);
  u_xlatu81 = (u_xlatu55 >> 6u);
  u_xlati55 = bitcast<i32>((u_xlatu81 ^ u_xlatu55));
  u_xlatu55 = (bitcast<u32>(u_xlati55) * 9u);
  u_xlatu81 = (u_xlatu55 >> 11u);
  u_xlati55 = bitcast<i32>((u_xlatu81 ^ u_xlatu55));
  u_xlati55 = (u_xlati55 * 32769i);
  u_xlati55 = bitcast<i32>((bitcast<u32>(u_xlati55) ^ u_xlatu5.y));
  u_xlatu55 = (bitcast<u32>(u_xlati55) * 1025u);
  u_xlatu81 = (u_xlatu55 >> 6u);
  u_xlati55 = bitcast<i32>((u_xlatu81 ^ u_xlatu55));
  u_xlatu55 = (bitcast<u32>(u_xlati55) * 9u);
  u_xlatu81 = (u_xlatu55 >> 11u);
  u_xlati55 = bitcast<i32>((u_xlatu81 ^ u_xlatu55));
  u_xlati55 = (u_xlati55 * 32769i);
  u_xlati55 = bitcast<i32>((bitcast<u32>(u_xlati55) ^ u_xlatu5.x));
  u_xlatu55 = (bitcast<u32>(u_xlati55) * 1025u);
  u_xlatu81 = (u_xlatu55 >> 6u);
  u_xlati55 = bitcast<i32>((u_xlatu81 ^ u_xlatu55));
  u_xlatu55 = (bitcast<u32>(u_xlati55) * 9u);
  u_xlatu81 = (u_xlatu55 >> 11u);
  u_xlati55 = bitcast<i32>((u_xlatu81 ^ u_xlatu55));
  u_xlati55 = (u_xlati55 * 32769i);
  param = 1065353216i;
  param_1 = u_xlati55;
  param_2 = 0i;
  param_3 = 23i;
  u_xlat55.x = bitcast<f32>(v_153(&(param), &(param_1), &(param_2), &(param_3)));
  u_xlat55.x = (u_xlat55.x + -1.0f);
  u_xlat81 = (-(u_xlat55.x) + 1.0f);
  if (u_xlatb29) {
    v_12 = u_xlat55.x;
  } else {
    v_12 = u_xlat81;
  }
  u_xlat29 = v_12;
  u_xlat79 = ((u_xlat79 * 2.0f) + -(u_xlat29));
  u_xlat29 = (u_xlat79 * u_xlat6.w);
  u_xlatb55 = (u_xlat29 >= 0.40000000596046447754f);
  let v_154 = u_xlatb55;
  u_xlat55.x = select(0.0f, u_xlat29, v_154);
  u_xlat79 = ((u_xlat6.w * u_xlat79) + -0.40000000596046447754f);
  u_xlat81 = dpdxCoarse(u_xlat29);
  u_xlat29 = dpdyCoarse(u_xlat29);
  u_xlat29 = (abs(u_xlat29) + abs(u_xlat81));
  u_xlat29 = max(u_xlat29, 0.00009999999747378752f);
  u_xlat79 = (u_xlat79 / u_xlat29);
  u_xlat79 = (u_xlat79 + 0.5f);
  u_xlat79 = clamp(u_xlat79, 0.0f, 1.0f);
  u_xlatb29 = !((v_1.m_1 == 0.0f));
  if (u_xlatb29) {
    v_13 = u_xlat79;
  } else {
    v_13 = u_xlat55.x;
  }
  u_xlat79 = v_13;
  u_xlat55.x = (u_xlat79 + -0.00009999999747378752f);
  u_xlatb55 = (u_xlat55.x < 0.0f);
  if (((select(0i, 1i, u_xlatb55) * -1i) != 0i)) {
    discard;
    return;
  }
  u_xlat26 = (u_xlat26 * u_xlat34.yyy);
  u_xlat26 = ((u_xlat34.xxx * vs_INTERP4.xyz) + u_xlat26);
  u_xlat26 = ((u_xlat34.zzz * vs_INTERP9) + u_xlat26);
  u_xlat55.x = dot(u_xlat26, u_xlat26);
  u_xlat55.x = inverseSqrt(u_xlat55.x);
  u_xlat26 = (u_xlat26 * u_xlat55.xxx);
  u_xlat55.x = (vs_INTERP8.y * v_1.unity_MatrixV[1i].z);
  u_xlat55.x = ((v_1.unity_MatrixV[0i].z * vs_INTERP8.x) + u_xlat55.x);
  u_xlat55.x = ((v_1.unity_MatrixV[2i].z * vs_INTERP8.z) + u_xlat55.x);
  u_xlat55.x = (u_xlat55.x + v_1.unity_MatrixV[3i].z);
  u_xlat55.x = (-(u_xlat55.x) + -(v_1.m_6.y));
  u_xlat55.x = max(u_xlat55.x, 0.0f);
  u_xlat55.x = (u_xlat55.x * v_1.unity_FogParams.x);
  u_xlat5 = textureSampleBias(unity_LightmapInd, samplerunity_Lightmap, vs_INTERP0, v_1.m.x);
  let v_155 = textureSampleBias(unity_Lightmap, samplerunity_Lightmap, vs_INTERP0, v_1.m.x).xyz;
  u_xlat6 = vec4<f32>(v_155.xyz, u_xlat6.w);
  let v_156 = (u_xlat5.xyz + vec3<f32>(-0.5f));
  u_xlat5 = vec4<f32>(v_156.xyz, u_xlat5.w);
  u_xlat81 = dot(u_xlat26, u_xlat5.xyz);
  u_xlat81 = (u_xlat81 + 0.5f);
  let v_157 = u_xlat81;
  let v_158 = (vec3<f32>(v_157, v_157, v_157) * u_xlat6.xyz);
  u_xlat5 = vec4<f32>(v_158.xyz, u_xlat5.w);
  u_xlat81 = max(u_xlat5.w, 0.00009999999747378752f);
  let v_159 = u_xlat5.xyz;
  let v_160 = u_xlat81;
  u_xlat5 = vec4<f32>(((v_159 / vec3<f32>(v_160, v_160, v_160))).xyz, u_xlat5.w);
  u_xlat8.x = u_xlat8.x;
  u_xlat8.x = clamp(u_xlat8.x, 0.0f, 1.0f);
  u_xlat79 = u_xlat79;
  u_xlat79 = clamp(u_xlat79, 0.0f, 1.0f);
  let v_161 = (u_xlat7 * vec3<f32>(0.95999997854232788086f));
  u_xlat6 = vec4<f32>(v_161.xyz, u_xlat6.w);
  u_xlat81 = (-(u_xlat8.x) + 1.0f);
  u_xlat82 = (u_xlat81 * u_xlat81);
  u_xlat82 = max(u_xlat82, 0.0078125f);
  u_xlat83 = (u_xlat82 * u_xlat82);
  u_xlat84 = (u_xlat8.x + 0.04000002145767211914f);
  u_xlat84 = min(u_xlat84, 1.0f);
  u_xlat7.x = ((u_xlat82 * 4.0f) + 2.0f);
  u_xlat33 = min(vs_INTERP6.w, 1.0f);
  if (u_xlatb0) {
    u_xlatb0 = (v_3.m_16.y == 1.0f);
    if (u_xlatb0) {
      u_xlat8 = (u_xlat2.xyxy + v_3.m_14);
      let v_162 = u_xlat8.xy;
      txVec30 = vec3<f32>(v_162.x, v_162.y, u_xlat2.z);
      let v_163 = txVec30;
      u_xlat9.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_163.xy, v_163.z);
      let v_164 = u_xlat8.zw;
      txVec31 = vec3<f32>(v_164.x, v_164.y, u_xlat2.z);
      let v_165 = txVec31;
      u_xlat9.y = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_165.xy, v_165.z);
      u_xlat8 = (u_xlat2.xyxy + v_3.m_15);
      let v_166 = u_xlat8.xy;
      txVec32 = vec3<f32>(v_166.x, v_166.y, u_xlat2.z);
      let v_167 = txVec32;
      u_xlat9.z = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_167.xy, v_167.z);
      let v_168 = u_xlat8.zw;
      txVec33 = vec3<f32>(v_168.x, v_168.y, u_xlat2.z);
      let v_169 = txVec33;
      u_xlat9.w = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_169.xy, v_169.z);
      u_xlat0.x = dot(u_xlat9, vec4<f32>(0.25f));
    } else {
      u_xlatb59 = (v_3.m_16.y == 2.0f);
      if (u_xlatb59) {
        u_xlat59 = ((u_xlat2.xy * v_3.m_17.zw) + vec2<f32>(0.5f));
        u_xlat59 = floor(u_xlat59);
        let v_170 = ((u_xlat2.xy * v_3.m_17.zw) + -(u_xlat59));
        u_xlat8 = vec4<f32>(v_170.xy, u_xlat8.zw);
        u_xlat9 = (u_xlat8.xxyy + vec4<f32>(0.5f, 1.0f, 0.5f, 1.0f));
        u_xlat10 = (u_xlat9.xxzz * u_xlat9.xxzz);
        u_xlat60 = (u_xlat10.yw * vec2<f32>(0.07999999821186065674f));
        let v_171 = ((u_xlat10.xz * vec2<f32>(0.5f)) + -(u_xlat8.xy));
        let v_172 = u_xlat9;
        u_xlat9 = vec4<f32>(v_171.x, v_172.y, v_171.y, v_172.w);
        let v_173 = (-(u_xlat8.xy) + vec2<f32>(1.0f));
        u_xlat10 = vec4<f32>(v_173.xy, u_xlat10.zw);
        u_xlat62 = min(u_xlat8.xy, vec2<f32>());
        u_xlat62 = ((-(u_xlat62) * u_xlat62) + u_xlat10.xy);
        let v_174 = max(u_xlat8.xy, vec2<f32>());
        u_xlat8 = vec4<f32>(v_174.xy, u_xlat8.zw);
        let v_175 = ((-(u_xlat8.xy) * u_xlat8.xy) + u_xlat9.yw);
        u_xlat8 = vec4<f32>(v_175.xy, u_xlat8.zw);
        u_xlat62 = (u_xlat62 + vec2<f32>(1.0f));
        let v_176 = (u_xlat8.xy + vec2<f32>(1.0f));
        u_xlat8 = vec4<f32>(v_176.xy, u_xlat8.zw);
        let v_177 = (u_xlat9.xz * vec2<f32>(0.15999999642372131348f));
        u_xlat11 = vec4<f32>(v_177.xy, u_xlat11.zw);
        let v_178 = (u_xlat10.xy * vec2<f32>(0.15999999642372131348f));
        u_xlat12 = vec4<f32>(v_178.xy, u_xlat12.zw);
        let v_179 = (u_xlat62 * vec2<f32>(0.15999999642372131348f));
        u_xlat10 = vec4<f32>(v_179.xy, u_xlat10.zw);
        let v_180 = (u_xlat8.xy * vec2<f32>(0.15999999642372131348f));
        u_xlat13 = vec4<f32>(v_180.xy, u_xlat13.zw);
        let v_181 = (u_xlat9.yw * vec2<f32>(0.15999999642372131348f));
        u_xlat8 = vec4<f32>(v_181.xy, u_xlat8.zw);
        u_xlat11.z = u_xlat10.x;
        u_xlat11.w = u_xlat8.x;
        u_xlat12.z = u_xlat13.x;
        u_xlat12.w = u_xlat60.x;
        u_xlat9 = (u_xlat11.zwxz + u_xlat12.zwxz);
        u_xlat10.z = u_xlat11.y;
        u_xlat10.w = u_xlat8.y;
        u_xlat13.z = u_xlat12.y;
        u_xlat13.w = u_xlat60.y;
        let v_182 = (u_xlat10.zyw + u_xlat13.zyw);
        u_xlat8 = vec4<f32>(v_182.xyz, u_xlat8.w);
        let v_183 = (u_xlat12.xzw / u_xlat9.zwy);
        u_xlat10 = vec4<f32>(v_183.xyz, u_xlat10.w);
        let v_184 = (u_xlat10.xyz + vec3<f32>(-2.5f, -0.5f, 1.5f));
        u_xlat10 = vec4<f32>(v_184.xyz, u_xlat10.w);
        let v_185 = (u_xlat13.zyw / u_xlat8.xyz);
        u_xlat11 = vec4<f32>(v_185.xyz, u_xlat11.w);
        let v_186 = (u_xlat11.xyz + vec3<f32>(-2.5f, -0.5f, 1.5f));
        u_xlat11 = vec4<f32>(v_186.xyz, u_xlat11.w);
        let v_187 = (u_xlat10.yxz * v_3.m_17.xxx);
        u_xlat10 = vec4<f32>(v_187.xyz, u_xlat10.w);
        let v_188 = (u_xlat11.xyz * v_3.m_17.yyy);
        u_xlat11 = vec4<f32>(v_188.xyz, u_xlat11.w);
        u_xlat10.w = u_xlat11.x;
        u_xlat12 = ((u_xlat59.xyxy * v_3.m_17.xyxy) + u_xlat10.ywxw);
        let v_189 = ((u_xlat59 * v_3.m_17.xy) + u_xlat10.zw);
        u_xlat13 = vec4<f32>(v_189.xy, u_xlat13.zw);
        u_xlat11.w = u_xlat10.y;
        let v_190 = u_xlat11.yz;
        let v_191 = u_xlat10;
        u_xlat10 = vec4<f32>(v_191.x, v_190.x, v_191.z, v_190.y);
        u_xlat14 = ((u_xlat59.xyxy * v_3.m_17.xyxy) + u_xlat10.xyzy);
        u_xlat11 = ((u_xlat59.xyxy * v_3.m_17.xyxy) + u_xlat11.wywz);
        u_xlat10 = ((u_xlat59.xyxy * v_3.m_17.xyxy) + u_xlat10.xwzw);
        u_xlat15 = (u_xlat8.xxxy * u_xlat9.zwyz);
        u_xlat16 = (u_xlat8.yyzz * u_xlat9);
        u_xlat59.x = (u_xlat8.z * u_xlat9.y);
        let v_192 = u_xlat12.xy;
        txVec34 = vec3<f32>(v_192.x, v_192.y, u_xlat2.z);
        let v_193 = txVec34;
        u_xlat85 = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_193.xy, v_193.z);
        let v_194 = u_xlat12.zw;
        txVec35 = vec3<f32>(v_194.x, v_194.y, u_xlat2.z);
        let v_195 = txVec35;
        u_xlat8.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_195.xy, v_195.z);
        u_xlat8.x = (u_xlat8.x * u_xlat15.y);
        u_xlat85 = ((u_xlat15.x * u_xlat85) + u_xlat8.x);
        let v_196 = u_xlat13.xy;
        txVec36 = vec3<f32>(v_196.x, v_196.y, u_xlat2.z);
        let v_197 = txVec36;
        u_xlat8.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_197.xy, v_197.z);
        u_xlat85 = ((u_xlat15.z * u_xlat8.x) + u_xlat85);
        let v_198 = u_xlat11.xy;
        txVec37 = vec3<f32>(v_198.x, v_198.y, u_xlat2.z);
        let v_199 = txVec37;
        u_xlat8.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_199.xy, v_199.z);
        u_xlat85 = ((u_xlat15.w * u_xlat8.x) + u_xlat85);
        let v_200 = u_xlat14.xy;
        txVec38 = vec3<f32>(v_200.x, v_200.y, u_xlat2.z);
        let v_201 = txVec38;
        u_xlat8.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_201.xy, v_201.z);
        u_xlat85 = ((u_xlat16.x * u_xlat8.x) + u_xlat85);
        let v_202 = u_xlat14.zw;
        txVec39 = vec3<f32>(v_202.x, v_202.y, u_xlat2.z);
        let v_203 = txVec39;
        u_xlat8.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_203.xy, v_203.z);
        u_xlat85 = ((u_xlat16.y * u_xlat8.x) + u_xlat85);
        let v_204 = u_xlat11.zw;
        txVec40 = vec3<f32>(v_204.x, v_204.y, u_xlat2.z);
        let v_205 = txVec40;
        u_xlat8.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_205.xy, v_205.z);
        u_xlat85 = ((u_xlat16.z * u_xlat8.x) + u_xlat85);
        let v_206 = u_xlat10.xy;
        txVec41 = vec3<f32>(v_206.x, v_206.y, u_xlat2.z);
        let v_207 = txVec41;
        u_xlat8.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_207.xy, v_207.z);
        u_xlat85 = ((u_xlat16.w * u_xlat8.x) + u_xlat85);
        let v_208 = u_xlat10.zw;
        txVec42 = vec3<f32>(v_208.x, v_208.y, u_xlat2.z);
        let v_209 = txVec42;
        u_xlat8.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_209.xy, v_209.z);
        u_xlat0.x = ((u_xlat59.x * u_xlat8.x) + u_xlat85);
      } else {
        u_xlat59 = ((u_xlat2.xy * v_3.m_17.zw) + vec2<f32>(0.5f));
        u_xlat59 = floor(u_xlat59);
        let v_210 = ((u_xlat2.xy * v_3.m_17.zw) + -(u_xlat59));
        u_xlat8 = vec4<f32>(v_210.xy, u_xlat8.zw);
        u_xlat9 = (u_xlat8.xxyy + vec4<f32>(0.5f, 1.0f, 0.5f, 1.0f));
        u_xlat10 = (u_xlat9.xxzz * u_xlat9.xxzz);
        let v_211 = (u_xlat10.yw * vec2<f32>(0.04081600159406661987f));
        let v_212 = u_xlat11;
        u_xlat11 = vec4<f32>(v_212.x, v_211.x, v_212.z, v_211.y);
        u_xlat60 = ((u_xlat10.xz * vec2<f32>(0.5f)) + -(u_xlat8.xy));
        let v_213 = (-(u_xlat8.xy) + vec2<f32>(1.0f));
        let v_214 = u_xlat9;
        u_xlat9 = vec4<f32>(v_213.x, v_214.y, v_213.y, v_214.w);
        let v_215 = min(u_xlat8.xy, vec2<f32>());
        u_xlat10 = vec4<f32>(v_215.xy, u_xlat10.zw);
        let v_216 = ((-(u_xlat10.xy) * u_xlat10.xy) + u_xlat9.xz);
        let v_217 = u_xlat9;
        u_xlat9 = vec4<f32>(v_216.x, v_217.y, v_216.y, v_217.w);
        let v_218 = max(u_xlat8.xy, vec2<f32>());
        u_xlat10 = vec4<f32>(v_218.xy, u_xlat10.zw);
        let v_219 = ((-(u_xlat10.xy) * u_xlat10.xy) + u_xlat9.yw);
        let v_220 = u_xlat9;
        u_xlat9 = vec4<f32>(v_220.x, v_219.x, v_220.z, v_219.y);
        u_xlat9 = (u_xlat9 + vec4<f32>(2.0f));
        u_xlat10.z = (u_xlat9.y * 0.08163200318813323975f);
        let v_221 = (u_xlat60.yx * vec2<f32>(0.08163200318813323975f));
        u_xlat12 = vec4<f32>(v_221.xy, u_xlat12.zw);
        u_xlat60 = (u_xlat9.xz * vec2<f32>(0.08163200318813323975f));
        u_xlat12.z = (u_xlat9.w * 0.08163200318813323975f);
        u_xlat10.x = u_xlat12.y;
        let v_222 = ((u_xlat8.xx * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.16326400637626647949f, 0.08163200318813323975f));
        let v_223 = u_xlat10;
        u_xlat10 = vec4<f32>(v_223.x, v_222.x, v_223.z, v_222.y);
        let v_224 = ((u_xlat8.xx * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.08163200318813323975f, 0.16326400637626647949f));
        let v_225 = u_xlat9;
        u_xlat9 = vec4<f32>(v_224.x, v_225.y, v_224.y, v_225.w);
        u_xlat9.y = u_xlat60.x;
        u_xlat9.w = u_xlat11.y;
        u_xlat10 = (u_xlat9 + u_xlat10);
        let v_226 = ((u_xlat8.yy * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.16326400637626647949f, 0.08163200318813323975f));
        let v_227 = u_xlat12;
        u_xlat12 = vec4<f32>(v_227.x, v_226.x, v_227.z, v_226.y);
        let v_228 = ((u_xlat8.yy * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.08163200318813323975f, 0.16326400637626647949f));
        let v_229 = u_xlat11;
        u_xlat11 = vec4<f32>(v_228.x, v_229.y, v_228.y, v_229.w);
        u_xlat11.y = u_xlat60.y;
        u_xlat8 = (u_xlat11 + u_xlat12);
        u_xlat9 = (u_xlat9 / u_xlat10);
        u_xlat9 = (u_xlat9 + vec4<f32>(-3.5f, -1.5f, 0.5f, 2.5f));
        u_xlat11 = (u_xlat11 / u_xlat8);
        u_xlat11 = (u_xlat11 + vec4<f32>(-3.5f, -1.5f, 0.5f, 2.5f));
        u_xlat9 = (u_xlat9.wxyz * v_3.m_17.xxxx);
        u_xlat11 = (u_xlat11.xwyz * v_3.m_17.yyyy);
        let v_230 = u_xlat9.yzw;
        u_xlat12 = vec4<f32>(v_230.x, u_xlat12.y, v_230.yz);
        u_xlat12.y = u_xlat11.x;
        u_xlat13 = ((u_xlat59.xyxy * v_3.m_17.xyxy) + u_xlat12.xyzy);
        let v_231 = ((u_xlat59 * v_3.m_17.xy) + u_xlat12.wy);
        u_xlat14 = vec4<f32>(v_231.xy, u_xlat14.zw);
        u_xlat9.y = u_xlat12.y;
        u_xlat12.y = u_xlat11.z;
        u_xlat15 = ((u_xlat59.xyxy * v_3.m_17.xyxy) + u_xlat12.xyzy);
        u_xlat66 = ((u_xlat59 * v_3.m_17.xy) + u_xlat12.wy);
        u_xlat9.z = u_xlat12.y;
        u_xlat16 = ((u_xlat59.xyxy * v_3.m_17.xyxy) + u_xlat9.xyxz);
        u_xlat12.y = u_xlat11.w;
        u_xlat17 = ((u_xlat59.xyxy * v_3.m_17.xyxy) + u_xlat12.xyzy);
        let v_232 = ((u_xlat59 * v_3.m_17.xy) + u_xlat12.wy);
        u_xlat35 = vec3<f32>(v_232.xy, u_xlat35.z);
        u_xlat9.w = u_xlat12.y;
        let v_233 = ((u_xlat59 * v_3.m_17.xy) + u_xlat9.xw);
        u_xlat18 = vec4<f32>(v_233.xy, u_xlat18.zw);
        let v_234 = u_xlat12.xzw;
        u_xlat11 = vec4<f32>(v_234.x, u_xlat11.y, v_234.yz);
        u_xlat12 = ((u_xlat59.xyxy * v_3.m_17.xyxy) + u_xlat11.xyzy);
        u_xlat63 = ((u_xlat59 * v_3.m_17.xy) + u_xlat11.wy);
        u_xlat11.x = u_xlat9.x;
        u_xlat59 = ((u_xlat59 * v_3.m_17.xy) + u_xlat11.xy);
        u_xlat19 = (u_xlat8.xxxx * u_xlat10);
        u_xlat20 = (u_xlat8.yyyy * u_xlat10);
        u_xlat21 = (u_xlat8.zzzz * u_xlat10);
        u_xlat8 = (u_xlat8.wwww * u_xlat10);
        let v_235 = u_xlat13.xy;
        txVec43 = vec3<f32>(v_235.x, v_235.y, u_xlat2.z);
        let v_236 = txVec43;
        u_xlat9.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_236.xy, v_236.z);
        let v_237 = u_xlat13.zw;
        txVec44 = vec3<f32>(v_237.x, v_237.y, u_xlat2.z);
        let v_238 = txVec44;
        u_xlat87 = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_238.xy, v_238.z);
        u_xlat87 = (u_xlat87 * u_xlat19.y);
        u_xlat9.x = ((u_xlat19.x * u_xlat9.x) + u_xlat87);
        let v_239 = u_xlat14.xy;
        txVec45 = vec3<f32>(v_239.x, v_239.y, u_xlat2.z);
        let v_240 = txVec45;
        u_xlat87 = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_240.xy, v_240.z);
        u_xlat9.x = ((u_xlat19.z * u_xlat87) + u_xlat9.x);
        let v_241 = u_xlat16.xy;
        txVec46 = vec3<f32>(v_241.x, v_241.y, u_xlat2.z);
        let v_242 = txVec46;
        u_xlat87 = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_242.xy, v_242.z);
        u_xlat9.x = ((u_xlat19.w * u_xlat87) + u_xlat9.x);
        let v_243 = u_xlat15.xy;
        txVec47 = vec3<f32>(v_243.x, v_243.y, u_xlat2.z);
        let v_244 = txVec47;
        u_xlat87 = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_244.xy, v_244.z);
        u_xlat9.x = ((u_xlat20.x * u_xlat87) + u_xlat9.x);
        let v_245 = u_xlat15.zw;
        txVec48 = vec3<f32>(v_245.x, v_245.y, u_xlat2.z);
        let v_246 = txVec48;
        u_xlat87 = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_246.xy, v_246.z);
        u_xlat9.x = ((u_xlat20.y * u_xlat87) + u_xlat9.x);
        let v_247 = u_xlat66;
        txVec49 = vec3<f32>(v_247.x, v_247.y, u_xlat2.z);
        let v_248 = txVec49;
        u_xlat87 = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_248.xy, v_248.z);
        u_xlat9.x = ((u_xlat20.z * u_xlat87) + u_xlat9.x);
        let v_249 = u_xlat16.zw;
        txVec50 = vec3<f32>(v_249.x, v_249.y, u_xlat2.z);
        let v_250 = txVec50;
        u_xlat87 = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_250.xy, v_250.z);
        u_xlat9.x = ((u_xlat20.w * u_xlat87) + u_xlat9.x);
        let v_251 = u_xlat17.xy;
        txVec51 = vec3<f32>(v_251.x, v_251.y, u_xlat2.z);
        let v_252 = txVec51;
        u_xlat87 = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_252.xy, v_252.z);
        u_xlat9.x = ((u_xlat21.x * u_xlat87) + u_xlat9.x);
        let v_253 = u_xlat17.zw;
        txVec52 = vec3<f32>(v_253.x, v_253.y, u_xlat2.z);
        let v_254 = txVec52;
        u_xlat87 = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_254.xy, v_254.z);
        u_xlat9.x = ((u_xlat21.y * u_xlat87) + u_xlat9.x);
        let v_255 = u_xlat35.xy;
        txVec53 = vec3<f32>(v_255.x, v_255.y, u_xlat2.z);
        let v_256 = txVec53;
        u_xlat35.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_256.xy, v_256.z);
        u_xlat9.x = ((u_xlat21.z * u_xlat35.x) + u_xlat9.x);
        let v_257 = u_xlat18.xy;
        txVec54 = vec3<f32>(v_257.x, v_257.y, u_xlat2.z);
        let v_258 = txVec54;
        u_xlat35.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_258.xy, v_258.z);
        u_xlat9.x = ((u_xlat21.w * u_xlat35.x) + u_xlat9.x);
        let v_259 = u_xlat12.xy;
        txVec55 = vec3<f32>(v_259.x, v_259.y, u_xlat2.z);
        let v_260 = txVec55;
        u_xlat35.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_260.xy, v_260.z);
        u_xlat8.x = ((u_xlat8.x * u_xlat35.x) + u_xlat9.x);
        let v_261 = u_xlat12.zw;
        txVec56 = vec3<f32>(v_261.x, v_261.y, u_xlat2.z);
        let v_262 = txVec56;
        u_xlat9.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_262.xy, v_262.z);
        u_xlat8.x = ((u_xlat8.y * u_xlat9.x) + u_xlat8.x);
        let v_263 = u_xlat63;
        txVec57 = vec3<f32>(v_263.x, v_263.y, u_xlat2.z);
        let v_264 = txVec57;
        u_xlat34.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_264.xy, v_264.z);
        u_xlat8.x = ((u_xlat8.z * u_xlat34.x) + u_xlat8.x);
        let v_265 = u_xlat59;
        txVec58 = vec3<f32>(v_265.x, v_265.y, u_xlat2.z);
        let v_266 = txVec58;
        u_xlat59.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_266.xy, v_266.z);
        u_xlat0.x = ((u_xlat8.w * u_xlat59.x) + u_xlat8.x);
      }
    }
  } else {
    let v_267 = u_xlat2.xy;
    txVec59 = vec3<f32>(v_267.x, v_267.y, u_xlat2.z);
    let v_268 = txVec59;
    u_xlat0.x = textureSampleCompareLevel(v_4, sampler_LinearClampCompare, v_268.xy, v_268.z);
  }
  u_xlat0.x = ((u_xlat0.x * v_3.m_16.x) + u_xlat80);
  if (u_xlatb3.x) {
    v_14 = 1.0f;
  } else {
    v_14 = u_xlat0.x;
  }
  u_xlat0.x = v_14;
  let v_269 = (vs_INTERP8 + -(v_1.m_5));
  u_xlat2 = vec4<f32>(v_269.xyz, u_xlat2.w);
  u_xlat2.x = dot(u_xlat2.xyz, u_xlat2.xyz);
  u_xlat28.x = ((u_xlat2.x * v_3.m_16.z) + v_3.m_16.w);
  u_xlat28.x = clamp(u_xlat28.x, 0.0f, 1.0f);
  u_xlat54 = (-(u_xlat0.x) + 1.0f);
  u_xlat0.x = ((u_xlat28.x * u_xlat54) + u_xlat0.x);
  u_xlatb28.x = !((v_5.m_26 == -1.0f));
  if (u_xlatb28.x) {
    let v_270 = (vs_INTERP8.yy * v_5.m_24[1i].xy);
    u_xlat28 = vec3<f32>(v_270.xy, u_xlat28.z);
    let v_271 = ((v_5.m_24[0i].xy * vs_INTERP8.xx) + u_xlat28.xy);
    u_xlat28 = vec3<f32>(v_271.xy, u_xlat28.z);
    let v_272 = ((v_5.m_24[2i].xy * vs_INTERP8.zz) + u_xlat28.xy);
    u_xlat28 = vec3<f32>(v_272.xy, u_xlat28.z);
    let v_273 = (u_xlat28.xy + v_5.m_24[3i].xy);
    u_xlat28 = vec3<f32>(v_273.xy, u_xlat28.z);
    let v_274 = ((u_xlat28.xy * vec2<f32>(0.5f)) + vec2<f32>(0.5f));
    u_xlat28 = vec3<f32>(v_274.xy, u_xlat28.z);
    u_xlat8 = textureSampleBias(v_6, sampler_MainLightCookieTexture, u_xlat28.xy, v_1.m.x);
    let v_275 = vec4<f32>(v_5.m_26, v_5.m_26, v_5.m_26, v_5.m_26);
    u_xlatb28 = ((vec4<f32>(v_275.x, v_275.y, v_275.z, v_275.w) == vec4<f32>(0.0f, 1.0f, 0.0f, 0.0f))).xy;
    if (u_xlatb28.y) {
      v_15 = u_xlat8.w;
    } else {
      v_15 = u_xlat8.x;
    }
    u_xlat54 = v_15;
    if (u_xlatb28.x) {
      v_16 = u_xlat8.xyz;
    } else {
      let v_276 = u_xlat54;
      v_16 = vec3<f32>(v_276, v_276, v_276);
    }
    u_xlat28 = v_16;
  } else {
    u_xlat28.x = 1.0f;
    u_xlat28.y = 1.0f;
    u_xlat28.z = 1.0f;
  }
  u_xlat28 = (u_xlat28 * v_1.m_3.xyz);
  u_xlat3.x = dot(-(u_xlat4), u_xlat26);
  u_xlat3.x = (u_xlat3.x + u_xlat3.x);
  let v_277 = ((u_xlat26 * -(u_xlat3.xxx)) + -(u_xlat4));
  u_xlat8 = vec4<f32>(v_277.xyz, u_xlat8.w);
  u_xlat3.x = dot(u_xlat26, u_xlat4);
  u_xlat3.x = clamp(u_xlat3.x, 0.0f, 1.0f);
  u_xlat3.x = (-(u_xlat3.x) + 1.0f);
  u_xlat3.x = (u_xlat3.x * u_xlat3.x);
  u_xlat3.x = (u_xlat3.x * u_xlat3.x);
  u_xlat59.x = ((-(u_xlat81) * 0.69999998807907104492f) + 1.70000004768371582031f);
  u_xlat81 = (u_xlat81 * u_xlat59.x);
  u_xlat81 = (u_xlat81 * 6.0f);
  u_xlat8 = textureSampleLevel(unity_SpecCube0, samplerunity_SpecCube0, u_xlat8.xyz, u_xlat81);
  u_xlat81 = (u_xlat8.w + -1.0f);
  u_xlat81 = ((v.unity_SpecCube0_HDR.w * u_xlat81) + 1.0f);
  u_xlat81 = max(u_xlat81, 0.0f);
  u_xlat81 = log2(u_xlat81);
  u_xlat81 = (u_xlat81 * v.unity_SpecCube0_HDR.y);
  u_xlat81 = exp2(u_xlat81);
  u_xlat81 = (u_xlat81 * v.unity_SpecCube0_HDR.x);
  let v_278 = u_xlat8.xyz;
  let v_279 = u_xlat81;
  u_xlat8 = vec4<f32>(((v_278 * vec3<f32>(v_279, v_279, v_279))).xyz, u_xlat8.w);
  let v_280 = u_xlat82;
  let v_281 = u_xlat82;
  u_xlat59 = ((vec2<f32>(v_280, v_280) * vec2<f32>(v_281, v_281)) + vec2<f32>(-1.0f, 1.0f));
  u_xlat81 = (1.0f / u_xlat59.y);
  u_xlat82 = (u_xlat84 + -0.03999999910593032837f);
  u_xlat3.x = ((u_xlat3.x * u_xlat82) + 0.03999999910593032837f);
  u_xlat3.x = (u_xlat3.x * u_xlat81);
  let v_282 = (u_xlat3.xxx * u_xlat8.xyz);
  u_xlat8 = vec4<f32>(v_282.xyz, u_xlat8.w);
  let v_283 = ((u_xlat5.xyz * u_xlat6.xyz) + u_xlat8.xyz);
  u_xlat5 = vec4<f32>(v_283.xyz, u_xlat5.w);
  u_xlat0.x = (u_xlat0.x * v.unity_LightData.z);
  u_xlat3.x = dot(u_xlat26, v_1.m_2.xyz);
  u_xlat3.x = clamp(u_xlat3.x, 0.0f, 1.0f);
  u_xlat0.x = (u_xlat0.x * u_xlat3.x);
  u_xlat28 = (u_xlat0.xxx * u_xlat28);
  let v_284 = (u_xlat4 + v_1.m_2.xyz);
  u_xlat8 = vec4<f32>(v_284.xyz, u_xlat8.w);
  u_xlat0.x = dot(u_xlat8.xyz, u_xlat8.xyz);
  u_xlat0.x = max(u_xlat0.x, 1.17549435e-38f);
  u_xlat0.x = inverseSqrt(u_xlat0.x);
  let v_285 = (u_xlat0.xxx * u_xlat8.xyz);
  u_xlat8 = vec4<f32>(v_285.xyz, u_xlat8.w);
  u_xlat0.x = dot(u_xlat26, u_xlat8.xyz);
  u_xlat0.x = clamp(u_xlat0.x, 0.0f, 1.0f);
  u_xlat3.x = dot(v_1.m_2.xyz, u_xlat8.xyz);
  u_xlat3.x = clamp(u_xlat3.x, 0.0f, 1.0f);
  u_xlat0.x = (u_xlat0.x * u_xlat0.x);
  u_xlat0.x = ((u_xlat0.x * u_xlat59.x) + 1.00001001358032226562f);
  u_xlat3.x = (u_xlat3.x * u_xlat3.x);
  u_xlat0.x = (u_xlat0.x * u_xlat0.x);
  u_xlat3.x = max(u_xlat3.x, 0.10000000149011611938f);
  u_xlat0.x = (u_xlat0.x * u_xlat3.x);
  u_xlat0.x = (u_xlat7.x * u_xlat0.x);
  u_xlat0.x = (u_xlat83 / u_xlat0.x);
  let v_286 = ((u_xlat0.xxx * vec3<f32>(0.03999999910593032837f)) + u_xlat6.xyz);
  u_xlat8 = vec4<f32>(v_286.xyz, u_xlat8.w);
  u_xlat28 = (u_xlat28 * u_xlat8.xyz);
  u_xlat0.x = min(v_1.m_4.x, v.unity_LightData.y);
  u_xlatu0 = bitcast<u32>(i32(u_xlat0.x));
  u_xlat2.x = ((u_xlat2.x * v_3.m_20.x) + v_3.m_20.y);
  u_xlat2.x = clamp(u_xlat2.x, 0.0f, 1.0f);
  let v_287 = vec4<f32>(v_5.m_27, v_5.m_27, v_5.m_27, v_5.m_27);
  let v_288 = ((vec4<f32>(v_287.x, v_287.y, v_287.z, v_287.w) == vec4<f32>(0.0f, 0.0f, 0.0f, 1.0f))).xw;
  u_xlatb3 = vec4<bool>(v_288.x, u_xlatb3.yz, v_288.y);
  u_xlat8.x = 0.0f;
  u_xlat8.y = 0.0f;
  u_xlat8.z = 0.0f;
  u_xlatu_loop_1 = 0u;
  loop {
    if ((u_xlatu_loop_1 < u_xlatu0)) {
      u_xlatu84 = (u_xlatu_loop_1 >> 2u);
      u_xlati85 = bitcast<i32>((u_xlatu_loop_1 & 3u));
      let v_289 = v.unity_LightIndices[bitcast<i32>(u_xlatu84)];
      let v_290 = u_xlati85;
      indexable = array<vec4<u32>, 4u>(vec4<u32>(1065353216u, 0u, 0u, 0u), vec4<u32>(0u, 1065353216u, 0u, 0u), vec4<u32>(0u, 0u, 1065353216u, 0u), vec4<u32>(0u, 0u, 0u, 1065353216u));
      u_xlat84 = dot(v_289, bitcast<vec4<f32>>(indexable[v_290]));
      u_xlati84 = i32(u_xlat84);
      let v_291 = ((-(vs_INTERP8) * v_7.m_31[u_xlati84].www) + v_7.m_31[u_xlati84].xyz);
      u_xlat9 = vec4<f32>(v_291.xyz, u_xlat9.w);
      u_xlat85 = dot(u_xlat9.xyz, u_xlat9.xyz);
      u_xlat85 = max(u_xlat85, 0.00006103515625f);
      u_xlat86 = inverseSqrt(u_xlat85);
      let v_292 = u_xlat86;
      let v_293 = (vec3<f32>(v_292, v_292, v_292) * u_xlat9.xyz);
      u_xlat10 = vec4<f32>(v_293.xyz, u_xlat10.w);
      u_xlat87 = (1.0f / u_xlat85);
      u_xlat85 = (u_xlat85 * v_7.m_33[u_xlati84].x);
      u_xlat85 = ((-(u_xlat85) * u_xlat85) + 1.0f);
      u_xlat85 = max(u_xlat85, 0.0f);
      u_xlat85 = (u_xlat85 * u_xlat85);
      u_xlat85 = (u_xlat85 * u_xlat87);
      u_xlat87 = dot(v_7.m_34[u_xlati84].xyz, u_xlat10.xyz);
      u_xlat87 = ((u_xlat87 * v_7.m_33[u_xlati84].z) + v_7.m_33[u_xlati84].w);
      u_xlat87 = clamp(u_xlat87, 0.0f, 1.0f);
      u_xlat87 = (u_xlat87 * u_xlat87);
      u_xlat85 = (u_xlat85 * u_xlat87);
      u_xlati87 = i32(v_3.m_22[u_xlati84].w);
      u_xlatb88 = (u_xlati87 >= 0i);
      if (u_xlatb88) {
        let v_294 = v_3.m_22[u_xlati84].z;
        u_xlatb88 = any(!((vec4<f32>() == vec4<f32>(v_294, v_294, v_294, v_294))));
        if (u_xlatb88) {
          let v_295 = ((abs(u_xlat10.zzyz) >= abs(u_xlat10.xyxx))).xyz;
          u_xlatb11 = vec4<bool>(v_295.xyz, u_xlatb11.w);
          u_xlatb88 = (u_xlatb11.y & u_xlatb11.x);
          let v_296 = ((-(u_xlat10.zyzx) < vec4<f32>())).xyw;
          u_xlatb11 = vec4<bool>(v_296.xy, u_xlatb11.z, v_296.z);
          u_xlat11.x = select(4.0f, 5.0f, u_xlatb11.x);
          u_xlat11.y = select(2.0f, 3.0f, u_xlatb11.y);
          u_xlat89 = select(0.0f, 1.0f, u_xlatb11.w);
          if (u_xlatb11.z) {
            v_17 = u_xlat11.y;
          } else {
            v_17 = u_xlat89;
          }
          u_xlat37.x = v_17;
          if (u_xlatb88) {
            v_18 = u_xlat11.x;
          } else {
            v_18 = u_xlat37.x;
          }
          u_xlat88 = v_18;
          u_xlat11.x = trunc(v_3.m_22[u_xlati84].w);
          u_xlat88 = (u_xlat88 + u_xlat11.x);
          u_xlati87 = i32(u_xlat88);
        }
        u_xlati87 = (u_xlati87 << bitcast<u32>(2i));
        u_xlat11 = (vs_INTERP8.yyyy * v_3.m_23[((u_xlati87 + 1i) / 4i)][((u_xlati87 + 1i) % 4i)]);
        u_xlat11 = ((v_3.m_23[(u_xlati87 / 4i)][(u_xlati87 % 4i)] * vs_INTERP8.xxxx) + u_xlat11);
        u_xlat11 = ((v_3.m_23[((u_xlati87 + 2i) / 4i)][((u_xlati87 + 2i) % 4i)] * vs_INTERP8.zzzz) + u_xlat11);
        u_xlat11 = (u_xlat11 + v_3.m_23[((u_xlati87 + 3i) / 4i)][((u_xlati87 + 3i) % 4i)]);
        let v_297 = (u_xlat11.xyz / u_xlat11.www);
        u_xlat11 = vec4<f32>(v_297.xyz, u_xlat11.w);
        u_xlatb87 = (0.0f < v_3.m_22[u_xlati84].y);
        if (u_xlatb87) {
          u_xlatb87 = (1.0f == v_3.m_22[u_xlati84].y);
          if (u_xlatb87) {
            u_xlat12 = (u_xlat11.xyxy + v_3.m_18);
            let v_298 = u_xlat12.xy;
            txVec60 = vec3<f32>(v_298.x, v_298.y, u_xlat11.z);
            let v_299 = txVec60;
            u_xlat13.x = textureSampleCompareLevel(v_8, sampler_LinearClampCompare, v_299.xy, v_299.z);
            let v_300 = u_xlat12.zw;
            txVec61 = vec3<f32>(v_300.x, v_300.y, u_xlat11.z);
            let v_301 = txVec61;
            u_xlat13.y = textureSampleCompareLevel(v_8, sampler_LinearClampCompare, v_301.xy, v_301.z);
            u_xlat12 = (u_xlat11.xyxy + v_3.m_19);
            let v_302 = u_xlat12.xy;
            txVec62 = vec3<f32>(v_302.x, v_302.y, u_xlat11.z);
            let v_303 = txVec62;
            u_xlat13.z = textureSampleCompareLevel(v_8, sampler_LinearClampCompare, v_303.xy, v_303.z);
            let v_304 = u_xlat12.zw;
            txVec63 = vec3<f32>(v_304.x, v_304.y, u_xlat11.z);
            let v_305 = txVec63;
            u_xlat13.w = textureSampleCompareLevel(v_8, sampler_LinearClampCompare, v_305.xy, v_305.z);
            u_xlat87 = dot(u_xlat13, vec4<f32>(0.25f));
          } else {
            u_xlatb88 = (2.0f == v_3.m_22[u_xlati84].y);
            if (u_xlatb88) {
              let v_306 = ((u_xlat11.xy * v_3.m_21.zw) + vec2<f32>(0.5f));
              u_xlat12 = vec4<f32>(v_306.xy, u_xlat12.zw);
              let v_307 = floor(u_xlat12.xy);
              u_xlat12 = vec4<f32>(v_307.xy, u_xlat12.zw);
              u_xlat64 = ((u_xlat11.xy * v_3.m_21.zw) + -(u_xlat12.xy));
              u_xlat13 = (u_xlat64.xxyy + vec4<f32>(0.5f, 1.0f, 0.5f, 1.0f));
              u_xlat14 = (u_xlat13.xxzz * u_xlat13.xxzz);
              let v_308 = (u_xlat14.yw * vec2<f32>(0.07999999821186065674f));
              let v_309 = u_xlat13;
              u_xlat13 = vec4<f32>(v_308.x, v_309.y, v_308.y, v_309.w);
              let v_310 = ((u_xlat14.xz * vec2<f32>(0.5f)) + -(u_xlat64));
              u_xlat14 = vec4<f32>(v_310.xy, u_xlat14.zw);
              u_xlat66 = (-(u_xlat64) + vec2<f32>(1.0f));
              let v_311 = min(u_xlat64, vec2<f32>());
              u_xlat15 = vec4<f32>(v_311.xy, u_xlat15.zw);
              let v_312 = ((-(u_xlat15.xy) * u_xlat15.xy) + u_xlat66);
              u_xlat15 = vec4<f32>(v_312.xy, u_xlat15.zw);
              u_xlat64 = max(u_xlat64, vec2<f32>());
              u_xlat64 = ((-(u_xlat64) * u_xlat64) + u_xlat13.yw);
              let v_313 = (u_xlat15.xy + vec2<f32>(1.0f));
              u_xlat15 = vec4<f32>(v_313.xy, u_xlat15.zw);
              u_xlat64 = (u_xlat64 + vec2<f32>(1.0f));
              let v_314 = (u_xlat14.xy * vec2<f32>(0.15999999642372131348f));
              u_xlat16 = vec4<f32>(v_314.xy, u_xlat16.zw);
              let v_315 = (u_xlat66 * vec2<f32>(0.15999999642372131348f));
              u_xlat14 = vec4<f32>(v_315.xy, u_xlat14.zw);
              let v_316 = (u_xlat15.xy * vec2<f32>(0.15999999642372131348f));
              u_xlat15 = vec4<f32>(v_316.xy, u_xlat15.zw);
              let v_317 = (u_xlat64 * vec2<f32>(0.15999999642372131348f));
              u_xlat17 = vec4<f32>(v_317.xy, u_xlat17.zw);
              u_xlat64 = (u_xlat13.yw * vec2<f32>(0.15999999642372131348f));
              u_xlat16.z = u_xlat15.x;
              u_xlat16.w = u_xlat64.x;
              u_xlat14.z = u_xlat17.x;
              u_xlat14.w = u_xlat13.x;
              u_xlat18 = (u_xlat14.zwxz + u_xlat16.zwxz);
              u_xlat15.z = u_xlat16.y;
              u_xlat15.w = u_xlat64.y;
              u_xlat17.z = u_xlat14.y;
              u_xlat17.w = u_xlat13.z;
              let v_318 = (u_xlat15.zyw + u_xlat17.zyw);
              u_xlat13 = vec4<f32>(v_318.xyz, u_xlat13.w);
              let v_319 = (u_xlat14.xzw / u_xlat18.zwy);
              u_xlat14 = vec4<f32>(v_319.xyz, u_xlat14.w);
              let v_320 = (u_xlat14.xyz + vec3<f32>(-2.5f, -0.5f, 1.5f));
              u_xlat14 = vec4<f32>(v_320.xyz, u_xlat14.w);
              let v_321 = (u_xlat17.zyw / u_xlat13.xyz);
              u_xlat15 = vec4<f32>(v_321.xyz, u_xlat15.w);
              let v_322 = (u_xlat15.xyz + vec3<f32>(-2.5f, -0.5f, 1.5f));
              u_xlat15 = vec4<f32>(v_322.xyz, u_xlat15.w);
              let v_323 = (u_xlat14.yxz * v_3.m_21.xxx);
              u_xlat14 = vec4<f32>(v_323.xyz, u_xlat14.w);
              let v_324 = (u_xlat15.xyz * v_3.m_21.yyy);
              u_xlat15 = vec4<f32>(v_324.xyz, u_xlat15.w);
              u_xlat14.w = u_xlat15.x;
              u_xlat16 = ((u_xlat12.xyxy * v_3.m_21.xyxy) + u_xlat14.ywxw);
              u_xlat64 = ((u_xlat12.xy * v_3.m_21.xy) + u_xlat14.zw);
              u_xlat15.w = u_xlat14.y;
              let v_325 = u_xlat15.yz;
              let v_326 = u_xlat14;
              u_xlat14 = vec4<f32>(v_326.x, v_325.x, v_326.z, v_325.y);
              u_xlat17 = ((u_xlat12.xyxy * v_3.m_21.xyxy) + u_xlat14.xyzy);
              u_xlat15 = ((u_xlat12.xyxy * v_3.m_21.xyxy) + u_xlat15.wywz);
              u_xlat14 = ((u_xlat12.xyxy * v_3.m_21.xyxy) + u_xlat14.xwzw);
              u_xlat19 = (u_xlat13.xxxy * u_xlat18.zwyz);
              u_xlat20 = (u_xlat13.yyzz * u_xlat18);
              u_xlat88 = (u_xlat13.z * u_xlat18.y);
              let v_327 = u_xlat16.xy;
              txVec64 = vec3<f32>(v_327.x, v_327.y, u_xlat11.z);
              let v_328 = txVec64;
              u_xlat89 = textureSampleCompareLevel(v_8, sampler_LinearClampCompare, v_328.xy, v_328.z);
              let v_329 = u_xlat16.zw;
              txVec65 = vec3<f32>(v_329.x, v_329.y, u_xlat11.z);
              let v_330 = txVec65;
              u_xlat12.x = textureSampleCompareLevel(v_8, sampler_LinearClampCompare, v_330.xy, v_330.z);
              u_xlat12.x = (u_xlat12.x * u_xlat19.y);
              u_xlat89 = ((u_xlat19.x * u_xlat89) + u_xlat12.x);
              let v_331 = u_xlat64;
              txVec66 = vec3<f32>(v_331.x, v_331.y, u_xlat11.z);
              let v_332 = txVec66;
              u_xlat12.x = textureSampleCompareLevel(v_8, sampler_LinearClampCompare, v_332.xy, v_332.z);
              u_xlat89 = ((u_xlat19.z * u_xlat12.x) + u_xlat89);
              let v_333 = u_xlat15.xy;
              txVec67 = vec3<f32>(v_333.x, v_333.y, u_xlat11.z);
              let v_334 = txVec67;
              u_xlat12.x = textureSampleCompareLevel(v_8, sampler_LinearClampCompare, v_334.xy, v_334.z);
              u_xlat89 = ((u_xlat19.w * u_xlat12.x) + u_xlat89);
              let v_335 = u_xlat17.xy;
              txVec68 = vec3<f32>(v_335.x, v_335.y, u_xlat11.z);
              let v_336 = txVec68;
              u_xlat12.x = textureSampleCompareLevel(v_8, sampler_LinearClampCompare, v_336.xy, v_336.z);
              u_xlat89 = ((u_xlat20.x * u_xlat12.x) + u_xlat89);
              let v_337 = u_xlat17.zw;
              txVec69 = vec3<f32>(v_337.x, v_337.y, u_xlat11.z);
              let v_338 = txVec69;
              u_xlat12.x = textureSampleCompareLevel(v_8, sampler_LinearClampCompare, v_338.xy, v_338.z);
              u_xlat89 = ((u_xlat20.y * u_xlat12.x) + u_xlat89);
              let v_339 = u_xlat15.zw;
              txVec70 = vec3<f32>(v_339.x, v_339.y, u_xlat11.z);
              let v_340 = txVec70;
              u_xlat12.x = textureSampleCompareLevel(v_8, sampler_LinearClampCompare, v_340.xy, v_340.z);
              u_xlat89 = ((u_xlat20.z * u_xlat12.x) + u_xlat89);
              let v_341 = u_xlat14.xy;
              txVec71 = vec3<f32>(v_341.x, v_341.y, u_xlat11.z);
              let v_342 = txVec71;
              u_xlat12.x = textureSampleCompareLevel(v_8, sampler_LinearClampCompare, v_342.xy, v_342.z);
              u_xlat89 = ((u_xlat20.w * u_xlat12.x) + u_xlat89);
              let v_343 = u_xlat14.zw;
              txVec72 = vec3<f32>(v_343.x, v_343.y, u_xlat11.z);
              let v_344 = txVec72;
              u_xlat12.x = textureSampleCompareLevel(v_8, sampler_LinearClampCompare, v_344.xy, v_344.z);
              u_xlat87 = ((u_xlat88 * u_xlat12.x) + u_xlat89);
            } else {
              let v_345 = ((u_xlat11.xy * v_3.m_21.zw) + vec2<f32>(0.5f));
              u_xlat12 = vec4<f32>(v_345.xy, u_xlat12.zw);
              let v_346 = floor(u_xlat12.xy);
              u_xlat12 = vec4<f32>(v_346.xy, u_xlat12.zw);
              u_xlat64 = ((u_xlat11.xy * v_3.m_21.zw) + -(u_xlat12.xy));
              u_xlat13 = (u_xlat64.xxyy + vec4<f32>(0.5f, 1.0f, 0.5f, 1.0f));
              u_xlat14 = (u_xlat13.xxzz * u_xlat13.xxzz);
              let v_347 = (u_xlat14.yw * vec2<f32>(0.04081600159406661987f));
              let v_348 = u_xlat15;
              u_xlat15 = vec4<f32>(v_348.x, v_347.x, v_348.z, v_347.y);
              let v_349 = ((u_xlat14.xz * vec2<f32>(0.5f)) + -(u_xlat64));
              let v_350 = u_xlat13;
              u_xlat13 = vec4<f32>(v_349.x, v_350.y, v_349.y, v_350.w);
              let v_351 = (-(u_xlat64) + vec2<f32>(1.0f));
              u_xlat14 = vec4<f32>(v_351.xy, u_xlat14.zw);
              u_xlat66 = min(u_xlat64, vec2<f32>());
              let v_352 = ((-(u_xlat66) * u_xlat66) + u_xlat14.xy);
              u_xlat14 = vec4<f32>(v_352.xy, u_xlat14.zw);
              u_xlat66 = max(u_xlat64, vec2<f32>());
              let v_353 = ((-(u_xlat66) * u_xlat66) + u_xlat13.yw);
              u_xlat39 = vec3<f32>(v_353.x, u_xlat39.y, v_353.y);
              let v_354 = (u_xlat14.xy + vec2<f32>(2.0f));
              u_xlat14 = vec4<f32>(v_354.xy, u_xlat14.zw);
              let v_355 = (u_xlat39.xz + vec2<f32>(2.0f));
              let v_356 = u_xlat13;
              u_xlat13 = vec4<f32>(v_356.x, v_355.x, v_356.z, v_355.y);
              u_xlat16.z = (u_xlat13.y * 0.08163200318813323975f);
              let v_357 = (u_xlat13.zxw * vec3<f32>(0.08163200318813323975f));
              u_xlat17 = vec4<f32>(v_357.xyz, u_xlat17.w);
              let v_358 = (u_xlat14.xy * vec2<f32>(0.08163200318813323975f));
              u_xlat13 = vec4<f32>(v_358.xy, u_xlat13.zw);
              u_xlat16.x = u_xlat17.y;
              let v_359 = ((u_xlat64.xx * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.16326400637626647949f, 0.08163200318813323975f));
              let v_360 = u_xlat16;
              u_xlat16 = vec4<f32>(v_360.x, v_359.x, v_360.z, v_359.y);
              let v_361 = ((u_xlat64.xx * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.08163200318813323975f, 0.16326400637626647949f));
              let v_362 = u_xlat14;
              u_xlat14 = vec4<f32>(v_361.x, v_362.y, v_361.y, v_362.w);
              u_xlat14.y = u_xlat13.x;
              u_xlat14.w = u_xlat15.y;
              u_xlat16 = (u_xlat14 + u_xlat16);
              let v_363 = ((u_xlat64.yy * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.16326400637626647949f, 0.08163200318813323975f));
              let v_364 = u_xlat17;
              u_xlat17 = vec4<f32>(v_364.x, v_363.x, v_364.z, v_363.y);
              let v_365 = ((u_xlat64.yy * vec2<f32>(-0.08163200318813323975f, 0.08163200318813323975f)) + vec2<f32>(0.08163200318813323975f, 0.16326400637626647949f));
              let v_366 = u_xlat15;
              u_xlat15 = vec4<f32>(v_365.x, v_366.y, v_365.y, v_366.w);
              u_xlat15.y = u_xlat13.y;
              u_xlat13 = (u_xlat15 + u_xlat17);
              u_xlat14 = (u_xlat14 / u_xlat16);
              u_xlat14 = (u_xlat14 + vec4<f32>(-3.5f, -1.5f, 0.5f, 2.5f));
              u_xlat15 = (u_xlat15 / u_xlat13);
              u_xlat15 = (u_xlat15 + vec4<f32>(-3.5f, -1.5f, 0.5f, 2.5f));
              u_xlat14 = (u_xlat14.wxyz * v_3.m_21.xxxx);
              u_xlat15 = (u_xlat15.xwyz * v_3.m_21.yyyy);
              let v_367 = u_xlat14.yzw;
              u_xlat17 = vec4<f32>(v_367.x, u_xlat17.y, v_367.yz);
              u_xlat17.y = u_xlat15.x;
              u_xlat18 = ((u_xlat12.xyxy * v_3.m_21.xyxy) + u_xlat17.xyzy);
              u_xlat64 = ((u_xlat12.xy * v_3.m_21.xy) + u_xlat17.wy);
              u_xlat14.y = u_xlat17.y;
              u_xlat17.y = u_xlat15.z;
              u_xlat19 = ((u_xlat12.xyxy * v_3.m_21.xyxy) + u_xlat17.xyzy);
              let v_368 = ((u_xlat12.xy * v_3.m_21.xy) + u_xlat17.wy);
              u_xlat20 = vec4<f32>(v_368.xy, u_xlat20.zw);
              u_xlat14.z = u_xlat17.y;
              u_xlat21 = ((u_xlat12.xyxy * v_3.m_21.xyxy) + u_xlat14.xyxz);
              u_xlat17.y = u_xlat15.w;
              u_xlat22 = ((u_xlat12.xyxy * v_3.m_21.xyxy) + u_xlat17.xyzy);
              u_xlat40 = ((u_xlat12.xy * v_3.m_21.xy) + u_xlat17.wy);
              u_xlat14.w = u_xlat17.y;
              u_xlat72 = ((u_xlat12.xy * v_3.m_21.xy) + u_xlat14.xw);
              let v_369 = u_xlat17.xzw;
              u_xlat15 = vec4<f32>(v_369.x, u_xlat15.y, v_369.yz);
              u_xlat17 = ((u_xlat12.xyxy * v_3.m_21.xyxy) + u_xlat15.xyzy);
              u_xlat67 = ((u_xlat12.xy * v_3.m_21.xy) + u_xlat15.wy);
              u_xlat15.x = u_xlat14.x;
              let v_370 = ((u_xlat12.xy * v_3.m_21.xy) + u_xlat15.xy);
              u_xlat12 = vec4<f32>(v_370.xy, u_xlat12.zw);
              u_xlat23 = (u_xlat13.xxxx * u_xlat16);
              u_xlat24 = (u_xlat13.yyyy * u_xlat16);
              u_xlat25 = (u_xlat13.zzzz * u_xlat16);
              u_xlat13 = (u_xlat13.wwww * u_xlat16);
              let v_371 = u_xlat18.xy;
              txVec73 = vec3<f32>(v_371.x, v_371.y, u_xlat11.z);
              let v_372 = txVec73;
              u_xlat88 = textureSampleCompareLevel(v_8, sampler_LinearClampCompare, v_372.xy, v_372.z);
              let v_373 = u_xlat18.zw;
              txVec74 = vec3<f32>(v_373.x, v_373.y, u_xlat11.z);
              let v_374 = txVec74;
              u_xlat89 = textureSampleCompareLevel(v_8, sampler_LinearClampCompare, v_374.xy, v_374.z);
              u_xlat89 = (u_xlat89 * u_xlat23.y);
              u_xlat88 = ((u_xlat23.x * u_xlat88) + u_xlat89);
              let v_375 = u_xlat64;
              txVec75 = vec3<f32>(v_375.x, v_375.y, u_xlat11.z);
              let v_376 = txVec75;
              u_xlat89 = textureSampleCompareLevel(v_8, sampler_LinearClampCompare, v_376.xy, v_376.z);
              u_xlat88 = ((u_xlat23.z * u_xlat89) + u_xlat88);
              let v_377 = u_xlat21.xy;
              txVec76 = vec3<f32>(v_377.x, v_377.y, u_xlat11.z);
              let v_378 = txVec76;
              u_xlat89 = textureSampleCompareLevel(v_8, sampler_LinearClampCompare, v_378.xy, v_378.z);
              u_xlat88 = ((u_xlat23.w * u_xlat89) + u_xlat88);
              let v_379 = u_xlat19.xy;
              txVec77 = vec3<f32>(v_379.x, v_379.y, u_xlat11.z);
              let v_380 = txVec77;
              u_xlat89 = textureSampleCompareLevel(v_8, sampler_LinearClampCompare, v_380.xy, v_380.z);
              u_xlat88 = ((u_xlat24.x * u_xlat89) + u_xlat88);
              let v_381 = u_xlat19.zw;
              txVec78 = vec3<f32>(v_381.x, v_381.y, u_xlat11.z);
              let v_382 = txVec78;
              u_xlat89 = textureSampleCompareLevel(v_8, sampler_LinearClampCompare, v_382.xy, v_382.z);
              u_xlat88 = ((u_xlat24.y * u_xlat89) + u_xlat88);
              let v_383 = u_xlat20.xy;
              txVec79 = vec3<f32>(v_383.x, v_383.y, u_xlat11.z);
              let v_384 = txVec79;
              u_xlat89 = textureSampleCompareLevel(v_8, sampler_LinearClampCompare, v_384.xy, v_384.z);
              u_xlat88 = ((u_xlat24.z * u_xlat89) + u_xlat88);
              let v_385 = u_xlat21.zw;
              txVec80 = vec3<f32>(v_385.x, v_385.y, u_xlat11.z);
              let v_386 = txVec80;
              u_xlat89 = textureSampleCompareLevel(v_8, sampler_LinearClampCompare, v_386.xy, v_386.z);
              u_xlat88 = ((u_xlat24.w * u_xlat89) + u_xlat88);
              let v_387 = u_xlat22.xy;
              txVec81 = vec3<f32>(v_387.x, v_387.y, u_xlat11.z);
              let v_388 = txVec81;
              u_xlat89 = textureSampleCompareLevel(v_8, sampler_LinearClampCompare, v_388.xy, v_388.z);
              u_xlat88 = ((u_xlat25.x * u_xlat89) + u_xlat88);
              let v_389 = u_xlat22.zw;
              txVec82 = vec3<f32>(v_389.x, v_389.y, u_xlat11.z);
              let v_390 = txVec82;
              u_xlat89 = textureSampleCompareLevel(v_8, sampler_LinearClampCompare, v_390.xy, v_390.z);
              u_xlat88 = ((u_xlat25.y * u_xlat89) + u_xlat88);
              let v_391 = u_xlat40;
              txVec83 = vec3<f32>(v_391.x, v_391.y, u_xlat11.z);
              let v_392 = txVec83;
              u_xlat89 = textureSampleCompareLevel(v_8, sampler_LinearClampCompare, v_392.xy, v_392.z);
              u_xlat88 = ((u_xlat25.z * u_xlat89) + u_xlat88);
              let v_393 = u_xlat72;
              txVec84 = vec3<f32>(v_393.x, v_393.y, u_xlat11.z);
              let v_394 = txVec84;
              u_xlat89 = textureSampleCompareLevel(v_8, sampler_LinearClampCompare, v_394.xy, v_394.z);
              u_xlat88 = ((u_xlat25.w * u_xlat89) + u_xlat88);
              let v_395 = u_xlat17.xy;
              txVec85 = vec3<f32>(v_395.x, v_395.y, u_xlat11.z);
              let v_396 = txVec85;
              u_xlat89 = textureSampleCompareLevel(v_8, sampler_LinearClampCompare, v_396.xy, v_396.z);
              u_xlat88 = ((u_xlat13.x * u_xlat89) + u_xlat88);
              let v_397 = u_xlat17.zw;
              txVec86 = vec3<f32>(v_397.x, v_397.y, u_xlat11.z);
              let v_398 = txVec86;
              u_xlat89 = textureSampleCompareLevel(v_8, sampler_LinearClampCompare, v_398.xy, v_398.z);
              u_xlat88 = ((u_xlat13.y * u_xlat89) + u_xlat88);
              let v_399 = u_xlat67;
              txVec87 = vec3<f32>(v_399.x, v_399.y, u_xlat11.z);
              let v_400 = txVec87;
              u_xlat89 = textureSampleCompareLevel(v_8, sampler_LinearClampCompare, v_400.xy, v_400.z);
              u_xlat88 = ((u_xlat13.z * u_xlat89) + u_xlat88);
              let v_401 = u_xlat12.xy;
              txVec88 = vec3<f32>(v_401.x, v_401.y, u_xlat11.z);
              let v_402 = txVec88;
              u_xlat89 = textureSampleCompareLevel(v_8, sampler_LinearClampCompare, v_402.xy, v_402.z);
              u_xlat87 = ((u_xlat13.w * u_xlat89) + u_xlat88);
            }
          }
        } else {
          let v_403 = u_xlat11.xy;
          txVec89 = vec3<f32>(v_403.x, v_403.y, u_xlat11.z);
          let v_404 = txVec89;
          u_xlat87 = textureSampleCompareLevel(v_8, sampler_LinearClampCompare, v_404.xy, v_404.z);
        }
        u_xlat88 = (1.0f + -(v_3.m_22[u_xlati84].x));
        u_xlat87 = ((u_xlat87 * v_3.m_22[u_xlati84].x) + u_xlat88);
        u_xlatb88 = (0.0f >= u_xlat11.z);
        u_xlatb11.x = (u_xlat11.z >= 1.0f);
        u_xlatb88 = (u_xlatb88 | u_xlatb11.x);
        let v_405 = u_xlatb88;
        u_xlat87 = select(u_xlat87, 1.0f, v_405);
      } else {
        u_xlat87 = 1.0f;
      }
      u_xlat88 = (-(u_xlat87) + 1.0f);
      u_xlat87 = ((u_xlat2.x * u_xlat88) + u_xlat87);
      u_xlati88 = (1i << bitcast<u32>((u_xlati84 & 31i)));
      u_xlati88 = bitcast<i32>((bitcast<u32>(u_xlati88) & bitcast<u32>(v_5.m_25)));
      if ((u_xlati88 != 0i)) {
        u_xlati88 = i32(v_5.m_30[u_xlati84].tint_element);
        u_xlati11 = select(1i, 0i, (u_xlati88 != 0i));
        u_xlati37 = (u_xlati84 << bitcast<u32>(2i));
        if ((u_xlati11 != 0i)) {
          let v_406 = (vs_INTERP8.yyy * v_5.m_28[((u_xlati37 + 1i) / 4i)][((u_xlati37 + 1i) % 4i)].xyw);
          u_xlat11 = vec4<f32>(v_406.x, u_xlat11.y, v_406.yz);
          let v_407 = ((v_5.m_28[(u_xlati37 / 4i)][(u_xlati37 % 4i)].xyw * vs_INTERP8.xxx) + u_xlat11.xzw);
          u_xlat11 = vec4<f32>(v_407.x, u_xlat11.y, v_407.yz);
          let v_408 = ((v_5.m_28[((u_xlati37 + 2i) / 4i)][((u_xlati37 + 2i) % 4i)].xyw * vs_INTERP8.zzz) + u_xlat11.xzw);
          u_xlat11 = vec4<f32>(v_408.x, u_xlat11.y, v_408.yz);
          let v_409 = (u_xlat11.xzw + v_5.m_28[((u_xlati37 + 3i) / 4i)][((u_xlati37 + 3i) % 4i)].xyw);
          u_xlat11 = vec4<f32>(v_409.x, u_xlat11.y, v_409.yz);
          let v_410 = (u_xlat11.xz / u_xlat11.ww);
          let v_411 = u_xlat11;
          u_xlat11 = vec4<f32>(v_410.x, v_411.y, v_410.y, v_411.w);
          let v_412 = ((u_xlat11.xz * vec2<f32>(0.5f)) + vec2<f32>(0.5f));
          let v_413 = u_xlat11;
          u_xlat11 = vec4<f32>(v_412.x, v_413.y, v_412.y, v_413.w);
          let v_414 = clamp(u_xlat11.xz, vec2<f32>(0.0f, 0.0f), vec2<f32>(1.0f, 1.0f));
          let v_415 = u_xlat11;
          u_xlat11 = vec4<f32>(v_414.x, v_415.y, v_414.y, v_415.w);
          let v_416 = ((v_5.m_29[u_xlati84].xy * u_xlat11.xz) + v_5.m_29[u_xlati84].zw);
          let v_417 = u_xlat11;
          u_xlat11 = vec4<f32>(v_416.x, v_417.y, v_416.y, v_417.w);
        } else {
          u_xlatb88 = (u_xlati88 == 1i);
          u_xlati88 = select(0i, 1i, u_xlatb88);
          if ((u_xlati88 != 0i)) {
            let v_418 = (vs_INTERP8.yy * v_5.m_28[((u_xlati37 + 1i) / 4i)][((u_xlati37 + 1i) % 4i)].xy);
            u_xlat12 = vec4<f32>(v_418.xy, u_xlat12.zw);
            let v_419 = ((v_5.m_28[(u_xlati37 / 4i)][(u_xlati37 % 4i)].xy * vs_INTERP8.xx) + u_xlat12.xy);
            u_xlat12 = vec4<f32>(v_419.xy, u_xlat12.zw);
            let v_420 = ((v_5.m_28[((u_xlati37 + 2i) / 4i)][((u_xlati37 + 2i) % 4i)].xy * vs_INTERP8.zz) + u_xlat12.xy);
            u_xlat12 = vec4<f32>(v_420.xy, u_xlat12.zw);
            let v_421 = (u_xlat12.xy + v_5.m_28[((u_xlati37 + 3i) / 4i)][((u_xlati37 + 3i) % 4i)].xy);
            u_xlat12 = vec4<f32>(v_421.xy, u_xlat12.zw);
            let v_422 = ((u_xlat12.xy * vec2<f32>(0.5f)) + vec2<f32>(0.5f));
            u_xlat12 = vec4<f32>(v_422.xy, u_xlat12.zw);
            let v_423 = fract(u_xlat12.xy);
            u_xlat12 = vec4<f32>(v_423.xy, u_xlat12.zw);
            let v_424 = ((v_5.m_29[u_xlati84].xy * u_xlat12.xy) + v_5.m_29[u_xlati84].zw);
            let v_425 = u_xlat11;
            u_xlat11 = vec4<f32>(v_424.x, v_425.y, v_424.y, v_425.w);
          } else {
            u_xlat12 = (vs_INTERP8.yyyy * v_5.m_28[((u_xlati37 + 1i) / 4i)][((u_xlati37 + 1i) % 4i)]);
            u_xlat12 = ((v_5.m_28[(u_xlati37 / 4i)][(u_xlati37 % 4i)] * vs_INTERP8.xxxx) + u_xlat12);
            u_xlat12 = ((v_5.m_28[((u_xlati37 + 2i) / 4i)][((u_xlati37 + 2i) % 4i)] * vs_INTERP8.zzzz) + u_xlat12);
            u_xlat12 = (u_xlat12 + v_5.m_28[((u_xlati37 + 3i) / 4i)][((u_xlati37 + 3i) % 4i)]);
            let v_426 = (u_xlat12.xyz / u_xlat12.www);
            u_xlat12 = vec4<f32>(v_426.xyz, u_xlat12.w);
            u_xlat88 = dot(u_xlat12.xyz, u_xlat12.xyz);
            u_xlat88 = inverseSqrt(u_xlat88);
            let v_427 = u_xlat88;
            let v_428 = (vec3<f32>(v_427, v_427, v_427) * u_xlat12.xyz);
            u_xlat12 = vec4<f32>(v_428.xyz, u_xlat12.w);
            u_xlat88 = dot(abs(u_xlat12.xyz), vec3<f32>(1.0f));
            u_xlat88 = max(u_xlat88, 0.00000099999999747524f);
            u_xlat88 = (1.0f / u_xlat88);
            let v_429 = u_xlat88;
            let v_430 = (vec3<f32>(v_429, v_429, v_429) * u_xlat12.zxy);
            u_xlat13 = vec4<f32>(v_430.xyz, u_xlat13.w);
            u_xlat13.x = -(u_xlat13.x);
            u_xlat13.x = clamp(u_xlat13.x, 0.0f, 1.0f);
            let v_431 = ((u_xlat13.yyzz >= vec4<f32>())).xz;
            u_xlatb37 = vec3<bool>(v_431.x, u_xlatb37.y, v_431.y);
            if (u_xlatb37.x) {
              v_19 = u_xlat13.x;
            } else {
              v_19 = -(u_xlat13.x);
            }
            u_xlat37.x = v_19;
            if (u_xlatb37.z) {
              v_20 = u_xlat13.x;
            } else {
              v_20 = -(u_xlat13.x);
            }
            u_xlat37.z = v_20;
            let v_432 = u_xlat12.xy;
            let v_433 = u_xlat88;
            let v_434 = ((v_432 * vec2<f32>(v_433, v_433)) + u_xlat37.xz);
            u_xlat37 = vec3<f32>(v_434.x, u_xlat37.y, v_434.y);
            let v_435 = ((u_xlat37.xz * vec2<f32>(0.5f)) + vec2<f32>(0.5f));
            u_xlat37 = vec3<f32>(v_435.x, u_xlat37.y, v_435.y);
            let v_436 = clamp(u_xlat37.xz, vec2<f32>(0.0f, 0.0f), vec2<f32>(1.0f, 1.0f));
            u_xlat37 = vec3<f32>(v_436.x, u_xlat37.y, v_436.y);
            let v_437 = ((v_5.m_29[u_xlati84].xy * u_xlat37.xz) + v_5.m_29[u_xlati84].zw);
            let v_438 = u_xlat11;
            u_xlat11 = vec4<f32>(v_437.x, v_438.y, v_437.y, v_438.w);
          }
        }
        u_xlat11 = textureSampleLevel(v_9, sampler_LinearClamp, u_xlat11.xz, 0.0f);
        if (u_xlatb3.w) {
          v_21 = u_xlat11.w;
        } else {
          v_21 = u_xlat11.x;
        }
        u_xlat88 = v_21;
        if (u_xlatb3.x) {
          v_22 = u_xlat11.xyz;
        } else {
          let v_439 = u_xlat88;
          v_22 = vec3<f32>(v_439, v_439, v_439);
        }
        let v_440 = v_22;
        u_xlat11 = vec4<f32>(v_440.xyz, u_xlat11.w);
      } else {
        u_xlat11.x = 1.0f;
        u_xlat11.y = 1.0f;
        u_xlat11.z = 1.0f;
      }
      let v_441 = (u_xlat11.xyz * v_7.m_32[u_xlati84].xyz);
      u_xlat11 = vec4<f32>(v_441.xyz, u_xlat11.w);
      u_xlat84 = (u_xlat85 * u_xlat87);
      u_xlat85 = dot(u_xlat26, u_xlat10.xyz);
      u_xlat85 = clamp(u_xlat85, 0.0f, 1.0f);
      u_xlat84 = (u_xlat84 * u_xlat85);
      let v_442 = u_xlat84;
      let v_443 = (vec3<f32>(v_442, v_442, v_442) * u_xlat11.xyz);
      u_xlat11 = vec4<f32>(v_443.xyz, u_xlat11.w);
      let v_444 = u_xlat9.xyz;
      let v_445 = u_xlat86;
      let v_446 = ((v_444 * vec3<f32>(v_445, v_445, v_445)) + u_xlat4);
      u_xlat9 = vec4<f32>(v_446.xyz, u_xlat9.w);
      u_xlat84 = dot(u_xlat9.xyz, u_xlat9.xyz);
      u_xlat84 = max(u_xlat84, 1.17549435e-38f);
      u_xlat84 = inverseSqrt(u_xlat84);
      let v_447 = u_xlat84;
      let v_448 = (vec3<f32>(v_447, v_447, v_447) * u_xlat9.xyz);
      u_xlat9 = vec4<f32>(v_448.xyz, u_xlat9.w);
      u_xlat84 = dot(u_xlat26, u_xlat9.xyz);
      u_xlat84 = clamp(u_xlat84, 0.0f, 1.0f);
      u_xlat85 = dot(u_xlat10.xyz, u_xlat9.xyz);
      u_xlat85 = clamp(u_xlat85, 0.0f, 1.0f);
      u_xlat84 = (u_xlat84 * u_xlat84);
      u_xlat84 = ((u_xlat84 * u_xlat59.x) + 1.00001001358032226562f);
      u_xlat85 = (u_xlat85 * u_xlat85);
      u_xlat84 = (u_xlat84 * u_xlat84);
      u_xlat85 = max(u_xlat85, 0.10000000149011611938f);
      u_xlat84 = (u_xlat84 * u_xlat85);
      u_xlat84 = (u_xlat7.x * u_xlat84);
      u_xlat84 = (u_xlat83 / u_xlat84);
      let v_449 = u_xlat84;
      let v_450 = ((vec3<f32>(v_449, v_449, v_449) * vec3<f32>(0.03999999910593032837f)) + u_xlat6.xyz);
      u_xlat9 = vec4<f32>(v_450.xyz, u_xlat9.w);
      let v_451 = ((u_xlat9.xyz * u_xlat11.xyz) + u_xlat8.xyz);
      u_xlat8 = vec4<f32>(v_451.xyz, u_xlat8.w);
      continue;
    } else {
      break;
    }

    continuing {
      u_xlatu_loop_1 = (u_xlatu_loop_1 + bitcast<u32>(1i));
    }
  }
  let v_452 = u_xlat5.xyz;
  let v_453 = u_xlat33;
  u_xlat0 = ((v_452 * vec3<f32>(v_453, v_453, v_453)) + u_xlat28);
  u_xlat0 = (u_xlat8.xyz + u_xlat0);
  u_xlat0 = ((vs_INTERP6.www * u_xlat1) + u_xlat0);
  u_xlat78 = (u_xlat55.x * -(u_xlat55.x));
  u_xlat78 = exp2(u_xlat78);
  u_xlat0 = (u_xlat0 + -(v_1.unity_FogColor.xyz));
  let v_454 = u_xlat78;
  let v_455 = ((vec3<f32>(v_454, v_454, v_454) * u_xlat0) + v_1.unity_FogColor.xyz);
  SV_Target0 = vec4<f32>(v_455.xyz, SV_Target0.w);
  let v_456 = u_xlatb29;
  SV_Target0.w = select(1.0f, u_xlat79, v_456);
}

fn v_153(base : ptr<function, i32>, insert : ptr<function, i32>, offset : ptr<function, i32>, bits : ptr<function, i32>) -> i32 {
  var mask : u32;
  mask = (~((4294967295u << bitcast<u32>(*(bits)))) << bitcast<u32>(*(offset)));
  return bitcast<i32>(((bitcast<u32>(*(base)) & ~(mask)) | ((bitcast<u32>(*(insert)) << bitcast<u32>(*(offset))) & mask)));
}

@fragment
fn main(@location(5u) vs_INTERP9 : vec3<f32>, @location(1u) vs_INTERP4 : vec4<f32>, @location(4u) vs_INTERP8 : vec3<f32>, @location(2u) vs_INTERP5 : vec4<f32>, @location(3u) vs_INTERP6 : vec4<f32>, @location(0u) vs_INTERP0 : vec2<f32>, @builtin(position) gl_FragCoord : vec4<f32>) -> @location(0u) vec4<f32> {
  main_inner(vs_INTERP9, vs_INTERP4, vs_INTERP8, vs_INTERP5, vs_INTERP6, vs_INTERP0, gl_FragCoord);
  return SV_Target0;
}
