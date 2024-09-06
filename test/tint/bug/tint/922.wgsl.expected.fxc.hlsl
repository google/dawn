int tint_ftoi(float v) {
  return ((v <= 2147483520.0f) ? ((v < -2147483648.0f) ? -2147483648 : int(v)) : 2147483647);
}

struct Mat4x4_ {
  float4 mx;
  float4 my;
  float4 mz;
  float4 mw;
};
struct Mat4x3_ {
  float4 mx;
  float4 my;
  float4 mz;
};
struct Mat4x2_ {
  float4 mx;
  float4 my;
};
struct VertexOutput {
  float4 v_Color;
  float2 v_TexCoord;
  float4 member;
};

cbuffer cbuffer_global : register(b0) {
  uint4 global[4];
};
cbuffer cbuffer_global1 : register(b1) {
  uint4 global1[3];
};
cbuffer cbuffer_global2 : register(b2) {
  uint4 global2[96];
};
static float3 a_Position1 = float3(0.0f, 0.0f, 0.0f);
static float2 a_UV1 = float2(0.0f, 0.0f);
static float4 a_Color1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float3 a_Normal1 = float3(0.0f, 0.0f, 0.0f);
static float a_PosMtxIdx1 = 0.0f;
static float4 v_Color = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float2 v_TexCoord = float2(0.0f, 0.0f);
static float4 gl_Position = float4(0.0f, 0.0f, 0.0f, 0.0f);

float3 Mat4x3GetCol0_(Mat4x3_ m) {
  Mat4x3_ m1 = (Mat4x3_)0;
  m1 = m;
  Mat4x3_ x_e2 = m1;
  Mat4x3_ x_e5 = m1;
  Mat4x3_ x_e8 = m1;
  return float3(x_e2.mx.x, x_e5.my.x, x_e8.mz.x);
}

float3 Mat4x3GetCol1_(Mat4x3_ m2) {
  Mat4x3_ m3 = (Mat4x3_)0;
  m3 = m2;
  Mat4x3_ x_e2 = m3;
  Mat4x3_ x_e5 = m3;
  Mat4x3_ x_e8 = m3;
  return float3(x_e2.mx.y, x_e5.my.y, x_e8.mz.y);
}

float3 Mat4x3GetCol2_(Mat4x3_ m4) {
  Mat4x3_ m5 = (Mat4x3_)0;
  m5 = m4;
  Mat4x3_ x_e2 = m5;
  Mat4x3_ x_e5 = m5;
  Mat4x3_ x_e8 = m5;
  return float3(x_e2.mx.z, x_e5.my.z, x_e8.mz.z);
}

float3 Mat4x3GetCol3_(Mat4x3_ m6) {
  Mat4x3_ m7 = (Mat4x3_)0;
  m7 = m6;
  Mat4x3_ x_e2 = m7;
  Mat4x3_ x_e5 = m7;
  Mat4x3_ x_e8 = m7;
  return float3(x_e2.mx.w, x_e5.my.w, x_e8.mz.w);
}

float4 Mul(Mat4x4_ m8, float4 v) {
  Mat4x4_ m9 = (Mat4x4_)0;
  float4 v1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  m9 = m8;
  v1 = v;
  Mat4x4_ x_e4 = m9;
  float4 x_e6 = v1;
  Mat4x4_ x_e8 = m9;
  float4 x_e10 = v1;
  Mat4x4_ x_e12 = m9;
  float4 x_e14 = v1;
  Mat4x4_ x_e16 = m9;
  float4 x_e18 = v1;
  return float4(dot(x_e4.mx, x_e6), dot(x_e8.my, x_e10), dot(x_e12.mz, x_e14), dot(x_e16.mw, x_e18));
}

float3 Mul1(Mat4x3_ m10, float4 v2) {
  Mat4x3_ m11 = (Mat4x3_)0;
  float4 v3 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  m11 = m10;
  v3 = v2;
  Mat4x3_ x_e4 = m11;
  float4 x_e6 = v3;
  Mat4x3_ x_e8 = m11;
  float4 x_e10 = v3;
  Mat4x3_ x_e12 = m11;
  float4 x_e14 = v3;
  return float3(dot(x_e4.mx, x_e6), dot(x_e8.my, x_e10), dot(x_e12.mz, x_e14));
}

float2 Mul2(Mat4x2_ m12, float4 v4) {
  Mat4x2_ m13 = (Mat4x2_)0;
  float4 v5 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  m13 = m12;
  v5 = v4;
  Mat4x2_ x_e4 = m13;
  float4 x_e6 = v5;
  Mat4x2_ x_e8 = m13;
  float4 x_e10 = v5;
  return float2(dot(x_e4.mx, x_e6), dot(x_e8.my, x_e10));
}

float4 Mul3(float3 v6, Mat4x3_ m14) {
  float3 v7 = float3(0.0f, 0.0f, 0.0f);
  Mat4x3_ m15 = (Mat4x3_)0;
  v7 = v6;
  m15 = m14;
  Mat4x3_ x_e5 = m15;
  float3 x_e6 = Mat4x3GetCol0_(x_e5);
  float3 x_e7 = v7;
  Mat4x3_ x_e10 = m15;
  float3 x_e11 = Mat4x3GetCol1_(x_e10);
  float3 x_e12 = v7;
  Mat4x3_ x_e15 = m15;
  float3 x_e16 = Mat4x3GetCol2_(x_e15);
  float3 x_e17 = v7;
  Mat4x3_ x_e20 = m15;
  float3 x_e21 = Mat4x3GetCol3_(x_e20);
  float3 x_e22 = v7;
  return float4(dot(x_e6, x_e7), dot(x_e11, x_e12), dot(x_e16, x_e17), dot(x_e21, x_e22));
}

Mat4x4_ x_Mat4x4_(float n) {
  float n1 = 0.0f;
  Mat4x4_ o = (Mat4x4_)0;
  n1 = n;
  float x_e4 = n1;
  o.mx = float4(x_e4, 0.0f, 0.0f, 0.0f);
  float x_e11 = n1;
  o.my = float4(0.0f, x_e11, 0.0f, 0.0f);
  float x_e18 = n1;
  o.mz = float4(0.0f, 0.0f, x_e18, 0.0f);
  float x_e25 = n1;
  o.mw = float4(0.0f, 0.0f, 0.0f, x_e25);
  Mat4x4_ x_e27 = o;
  return x_e27;
}

Mat4x4_ x_Mat4x4_1(Mat4x3_ m16) {
  Mat4x3_ m17 = (Mat4x3_)0;
  Mat4x4_ o1 = (Mat4x4_)0;
  m17 = m16;
  Mat4x4_ x_e4 = x_Mat4x4_(1.0f);
  o1 = x_e4;
  Mat4x3_ x_e7 = m17;
  o1.mx = x_e7.mx;
  Mat4x3_ x_e10 = m17;
  o1.my = x_e10.my;
  Mat4x3_ x_e13 = m17;
  o1.mz = x_e13.mz;
  Mat4x4_ x_e15 = o1;
  return x_e15;
}

Mat4x4_ x_Mat4x4_2(Mat4x2_ m18) {
  Mat4x2_ m19 = (Mat4x2_)0;
  Mat4x4_ o2 = (Mat4x4_)0;
  m19 = m18;
  Mat4x4_ x_e4 = x_Mat4x4_(1.0f);
  o2 = x_e4;
  Mat4x2_ x_e7 = m19;
  o2.mx = x_e7.mx;
  Mat4x2_ x_e10 = m19;
  o2.my = x_e10.my;
  Mat4x4_ x_e12 = o2;
  return x_e12;
}

Mat4x3_ x_Mat4x3_(float n2) {
  float n3 = 0.0f;
  Mat4x3_ o3 = (Mat4x3_)0;
  n3 = n2;
  float x_e4 = n3;
  o3.mx = float4(x_e4, 0.0f, 0.0f, 0.0f);
  float x_e11 = n3;
  o3.my = float4(0.0f, x_e11, 0.0f, 0.0f);
  float x_e18 = n3;
  o3.mz = float4(0.0f, 0.0f, x_e18, 0.0f);
  Mat4x3_ x_e21 = o3;
  return x_e21;
}

Mat4x3_ x_Mat4x3_1(Mat4x4_ m20) {
  Mat4x4_ m21 = (Mat4x4_)0;
  Mat4x3_ o4 = (Mat4x3_)0;
  m21 = m20;
  Mat4x4_ x_e4 = m21;
  o4.mx = x_e4.mx;
  Mat4x4_ x_e7 = m21;
  o4.my = x_e7.my;
  Mat4x4_ x_e10 = m21;
  o4.mz = x_e10.mz;
  Mat4x3_ x_e12 = o4;
  return x_e12;
}

Mat4x3_ global2_load(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  Mat4x3_ tint_symbol_3 = {asfloat(global2[scalar_offset / 4]), asfloat(global2[scalar_offset_1 / 4]), asfloat(global2[scalar_offset_2 / 4])};
  return tint_symbol_3;
}

Mat4x4_ global_load(uint offset) {
  const uint scalar_offset_3 = ((offset + 0u)) / 4;
  const uint scalar_offset_4 = ((offset + 16u)) / 4;
  const uint scalar_offset_5 = ((offset + 32u)) / 4;
  const uint scalar_offset_6 = ((offset + 48u)) / 4;
  Mat4x4_ tint_symbol_4 = {asfloat(global[scalar_offset_3 / 4]), asfloat(global[scalar_offset_4 / 4]), asfloat(global[scalar_offset_5 / 4]), asfloat(global[scalar_offset_6 / 4])};
  return tint_symbol_4;
}

Mat4x2_ global1_load_1(uint offset) {
  const uint scalar_offset_7 = ((offset + 0u)) / 4;
  const uint scalar_offset_8 = ((offset + 16u)) / 4;
  Mat4x2_ tint_symbol_5 = {asfloat(global1[scalar_offset_7 / 4]), asfloat(global1[scalar_offset_8 / 4])};
  return tint_symbol_5;
}

void main1() {
  Mat4x3_ t_PosMtx = (Mat4x3_)0;
  float2 t_TexSpaceCoord = float2(0.0f, 0.0f);
  float x_e15 = a_PosMtxIdx1;
  Mat4x3_ x_e18 = global2_load((48u * uint(tint_ftoi(x_e15))));
  t_PosMtx = x_e18;
  Mat4x3_ x_e23 = t_PosMtx;
  Mat4x4_ x_e24 = x_Mat4x4_1(x_e23);
  float3 x_e25 = a_Position1;
  Mat4x3_ x_e29 = t_PosMtx;
  Mat4x4_ x_e30 = x_Mat4x4_1(x_e29);
  float3 x_e31 = a_Position1;
  float4 x_e34 = Mul(x_e30, float4(x_e31, 1.0f));
  Mat4x4_ x_e35 = global_load(0u);
  Mat4x3_ x_e37 = t_PosMtx;
  Mat4x4_ x_e38 = x_Mat4x4_1(x_e37);
  float3 x_e39 = a_Position1;
  Mat4x3_ x_e43 = t_PosMtx;
  Mat4x4_ x_e44 = x_Mat4x4_1(x_e43);
  float3 x_e45 = a_Position1;
  float4 x_e48 = Mul(x_e44, float4(x_e45, 1.0f));
  float4 x_e49 = Mul(x_e35, x_e48);
  gl_Position = x_e49;
  float4 x_e50 = a_Color1;
  v_Color = x_e50;
  float4 x_e52 = asfloat(global1[2]);
  if ((x_e52.x == 2.0f)) {
    {
      float3 x_e59 = a_Normal1;
      Mat4x2_ x_e64 = global1_load_1(0u);
      float3 x_e65 = a_Normal1;
      float2 x_e68 = Mul2(x_e64, float4(x_e65, 1.0f));
      v_TexCoord = x_e68.xy;
      return;
    }
  } else {
    {
      float2 x_e73 = a_UV1;
      Mat4x2_ x_e79 = global1_load_1(0u);
      float2 x_e80 = a_UV1;
      float2 x_e84 = Mul2(x_e79, float4(x_e80, 1.0f, 1.0f));
      v_TexCoord = x_e84.xy;
      return;
    }
  }
}

struct tint_symbol_1 {
  float3 a_Position : TEXCOORD0;
  float2 a_UV : TEXCOORD1;
  float4 a_Color : TEXCOORD2;
  float3 a_Normal : TEXCOORD3;
  float a_PosMtxIdx : TEXCOORD4;
};
struct tint_symbol_2 {
  float4 v_Color : TEXCOORD0;
  float2 v_TexCoord : TEXCOORD1;
  float4 member : SV_Position;
};

VertexOutput main_inner(float3 a_Position, float2 a_UV, float4 a_Color, float3 a_Normal, float a_PosMtxIdx) {
  a_Position1 = a_Position;
  a_UV1 = a_UV;
  a_Color1 = a_Color;
  a_Normal1 = a_Normal;
  a_PosMtxIdx1 = a_PosMtxIdx;
  main1();
  float4 x_e11 = v_Color;
  float2 x_e13 = v_TexCoord;
  float4 x_e15 = gl_Position;
  VertexOutput tint_symbol_6 = {x_e11, x_e13, x_e15};
  return tint_symbol_6;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  VertexOutput inner_result = main_inner(tint_symbol.a_Position, tint_symbol.a_UV, tint_symbol.a_Color, tint_symbol.a_Normal, tint_symbol.a_PosMtxIdx);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.v_Color = inner_result.v_Color;
  wrapper_result.v_TexCoord = inner_result.v_TexCoord;
  wrapper_result.member = inner_result.member;
  return wrapper_result;
}
