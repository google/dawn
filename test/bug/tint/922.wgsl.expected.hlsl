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

Mat4x4_ tint_symbol_7(uint4 buffer[4], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  const Mat4x4_ tint_symbol_10 = {asfloat(buffer[scalar_offset / 4]), asfloat(buffer[scalar_offset_1 / 4]), asfloat(buffer[scalar_offset_2 / 4]), asfloat(buffer[scalar_offset_3 / 4])};
  return tint_symbol_10;
}

Mat4x2_ tint_symbol_9(uint4 buffer[3], uint offset) {
  const uint scalar_offset_4 = ((offset + 0u)) / 4;
  const uint scalar_offset_5 = ((offset + 16u)) / 4;
  const Mat4x2_ tint_symbol_11 = {asfloat(buffer[scalar_offset_4 / 4]), asfloat(buffer[scalar_offset_5 / 4])};
  return tint_symbol_11;
}

Mat4x3_ tint_symbol_5(uint4 buffer[96], uint offset) {
  const uint scalar_offset_6 = ((offset + 0u)) / 4;
  const uint scalar_offset_7 = ((offset + 16u)) / 4;
  const uint scalar_offset_8 = ((offset + 32u)) / 4;
  const Mat4x3_ tint_symbol_12 = {asfloat(buffer[scalar_offset_6 / 4]), asfloat(buffer[scalar_offset_7 / 4]), asfloat(buffer[scalar_offset_8 / 4])};
  return tint_symbol_12;
}

struct VertexOutput {
  float4 v_Color;
  float2 v_TexCoord;
  float4 member;
};

cbuffer cbuffer_global : register(b0, space0) {
  uint4 global[4];
};
cbuffer cbuffer_global1 : register(b1, space0) {
  uint4 global1[3];
};
cbuffer cbuffer_global2 : register(b2, space0) {
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
  const Mat4x3_ _e2 = m1;
  const Mat4x3_ _e5 = m1;
  const Mat4x3_ _e8 = m1;
  return float3(_e2.mx.x, _e5.my.x, _e8.mz.x);
}

float3 Mat4x3GetCol1_(Mat4x3_ m2) {
  Mat4x3_ m3 = (Mat4x3_)0;
  m3 = m2;
  const Mat4x3_ _e2 = m3;
  const Mat4x3_ _e5 = m3;
  const Mat4x3_ _e8 = m3;
  return float3(_e2.mx.y, _e5.my.y, _e8.mz.y);
}

float3 Mat4x3GetCol2_(Mat4x3_ m4) {
  Mat4x3_ m5 = (Mat4x3_)0;
  m5 = m4;
  const Mat4x3_ _e2 = m5;
  const Mat4x3_ _e5 = m5;
  const Mat4x3_ _e8 = m5;
  return float3(_e2.mx.z, _e5.my.z, _e8.mz.z);
}

float3 Mat4x3GetCol3_(Mat4x3_ m6) {
  Mat4x3_ m7 = (Mat4x3_)0;
  m7 = m6;
  const Mat4x3_ _e2 = m7;
  const Mat4x3_ _e5 = m7;
  const Mat4x3_ _e8 = m7;
  return float3(_e2.mx.w, _e5.my.w, _e8.mz.w);
}

float4 Mul(Mat4x4_ m8, float4 v) {
  Mat4x4_ m9 = (Mat4x4_)0;
  float4 v1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  m9 = m8;
  v1 = v;
  const Mat4x4_ _e4 = m9;
  const float4 _e6 = v1;
  const Mat4x4_ _e8 = m9;
  const float4 _e10 = v1;
  const Mat4x4_ _e12 = m9;
  const float4 _e14 = v1;
  const Mat4x4_ _e16 = m9;
  const float4 _e18 = v1;
  return float4(dot(_e4.mx, _e6), dot(_e8.my, _e10), dot(_e12.mz, _e14), dot(_e16.mw, _e18));
}

float3 Mul1(Mat4x3_ m10, float4 v2) {
  Mat4x3_ m11 = (Mat4x3_)0;
  float4 v3 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  m11 = m10;
  v3 = v2;
  const Mat4x3_ _e4 = m11;
  const float4 _e6 = v3;
  const Mat4x3_ _e8 = m11;
  const float4 _e10 = v3;
  const Mat4x3_ _e12 = m11;
  const float4 _e14 = v3;
  return float3(dot(_e4.mx, _e6), dot(_e8.my, _e10), dot(_e12.mz, _e14));
}

float2 Mul2(Mat4x2_ m12, float4 v4) {
  Mat4x2_ m13 = (Mat4x2_)0;
  float4 v5 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  m13 = m12;
  v5 = v4;
  const Mat4x2_ _e4 = m13;
  const float4 _e6 = v5;
  const Mat4x2_ _e8 = m13;
  const float4 _e10 = v5;
  return float2(dot(_e4.mx, _e6), dot(_e8.my, _e10));
}

float4 Mul3(float3 v6, Mat4x3_ m14) {
  float3 v7 = float3(0.0f, 0.0f, 0.0f);
  Mat4x3_ m15 = (Mat4x3_)0;
  v7 = v6;
  m15 = m14;
  const Mat4x3_ _e5 = m15;
  const float3 _e6 = Mat4x3GetCol0_(_e5);
  const float3 _e7 = v7;
  const Mat4x3_ _e10 = m15;
  const float3 _e11 = Mat4x3GetCol1_(_e10);
  const float3 _e12 = v7;
  const Mat4x3_ _e15 = m15;
  const float3 _e16 = Mat4x3GetCol2_(_e15);
  const float3 _e17 = v7;
  const Mat4x3_ _e20 = m15;
  const float3 _e21 = Mat4x3GetCol3_(_e20);
  const float3 _e22 = v7;
  return float4(dot(_e6, _e7), dot(_e11, _e12), dot(_e16, _e17), dot(_e21, _e22));
}

Mat4x4_ _Mat4x4_(float n) {
  float n1 = 0.0f;
  Mat4x4_ o = (Mat4x4_)0;
  n1 = n;
  const float _e4 = n1;
  o.mx = float4(_e4, 0.0f, 0.0f, 0.0f);
  const float _e11 = n1;
  o.my = float4(0.0f, _e11, 0.0f, 0.0f);
  const float _e18 = n1;
  o.mz = float4(0.0f, 0.0f, _e18, 0.0f);
  const float _e25 = n1;
  o.mw = float4(0.0f, 0.0f, 0.0f, _e25);
  const Mat4x4_ _e27 = o;
  return _e27;
}

Mat4x4_ _Mat4x4_1(Mat4x3_ m16) {
  Mat4x3_ m17 = (Mat4x3_)0;
  Mat4x4_ o1 = (Mat4x4_)0;
  m17 = m16;
  const Mat4x4_ _e4 = _Mat4x4_(1.0f);
  o1 = _e4;
  const Mat4x3_ _e7 = m17;
  o1.mx = _e7.mx;
  const Mat4x3_ _e10 = m17;
  o1.my = _e10.my;
  const Mat4x3_ _e13 = m17;
  o1.mz = _e13.mz;
  const Mat4x4_ _e15 = o1;
  return _e15;
}

Mat4x4_ _Mat4x4_2(Mat4x2_ m18) {
  Mat4x2_ m19 = (Mat4x2_)0;
  Mat4x4_ o2 = (Mat4x4_)0;
  m19 = m18;
  const Mat4x4_ _e4 = _Mat4x4_(1.0f);
  o2 = _e4;
  const Mat4x2_ _e7 = m19;
  o2.mx = _e7.mx;
  const Mat4x2_ _e10 = m19;
  o2.my = _e10.my;
  const Mat4x4_ _e12 = o2;
  return _e12;
}

Mat4x3_ _Mat4x3_(float n2) {
  float n3 = 0.0f;
  Mat4x3_ o3 = (Mat4x3_)0;
  n3 = n2;
  const float _e4 = n3;
  o3.mx = float4(_e4, 0.0f, 0.0f, 0.0f);
  const float _e11 = n3;
  o3.my = float4(0.0f, _e11, 0.0f, 0.0f);
  const float _e18 = n3;
  o3.mz = float4(0.0f, 0.0f, _e18, 0.0f);
  const Mat4x3_ _e21 = o3;
  return _e21;
}

Mat4x3_ _Mat4x3_1(Mat4x4_ m20) {
  Mat4x4_ m21 = (Mat4x4_)0;
  Mat4x3_ o4 = (Mat4x3_)0;
  m21 = m20;
  const Mat4x4_ _e4 = m21;
  o4.mx = _e4.mx;
  const Mat4x4_ _e7 = m21;
  o4.my = _e7.my;
  const Mat4x4_ _e10 = m21;
  o4.mz = _e10.mz;
  const Mat4x3_ _e12 = o4;
  return _e12;
}

void main1() {
  Mat4x3_ t_PosMtx = (Mat4x3_)0;
  float2 t_TexSpaceCoord = float2(0.0f, 0.0f);
  const float _e15 = a_PosMtxIdx1;
  const Mat4x3_ _e18 = tint_symbol_5(global2, (48u * uint(int(_e15))));
  t_PosMtx = _e18;
  const Mat4x3_ _e23 = t_PosMtx;
  const Mat4x4_ _e24 = _Mat4x4_1(_e23);
  const float3 _e25 = a_Position1;
  const Mat4x3_ _e29 = t_PosMtx;
  const Mat4x4_ _e30 = _Mat4x4_1(_e29);
  const float3 _e31 = a_Position1;
  const float4 _e34 = Mul(_e30, float4(_e31, 1.0f));
  const Mat4x4_ _e35 = tint_symbol_7(global, 0u);
  const Mat4x3_ _e37 = t_PosMtx;
  const Mat4x4_ _e38 = _Mat4x4_1(_e37);
  const float3 _e39 = a_Position1;
  const Mat4x3_ _e43 = t_PosMtx;
  const Mat4x4_ _e44 = _Mat4x4_1(_e43);
  const float3 _e45 = a_Position1;
  const float4 _e48 = Mul(_e44, float4(_e45, 1.0f));
  const float4 _e49 = Mul(_e35, _e48);
  gl_Position = _e49;
  const float4 _e50 = a_Color1;
  v_Color = _e50;
  const uint scalar_offset_9 = (32u) / 4;
  const float4 _e52 = asfloat(global1[scalar_offset_9 / 4]);
  if ((_e52.x == 2.0f)) {
    {
      const float3 _e59 = a_Normal1;
      const Mat4x2_ _e64 = tint_symbol_9(global1, (32u * uint(0)));
      const float3 _e65 = a_Normal1;
      const float2 _e68 = Mul2(_e64, float4(_e65, 1.0f));
      v_TexCoord = _e68.xy;
      return;
    }
  } else {
    {
      const float2 _e73 = a_UV1;
      const Mat4x2_ _e79 = tint_symbol_9(global1, (32u * uint(0)));
      const float2 _e80 = a_UV1;
      const float2 _e84 = Mul2(_e79, float4(_e80, 1.0f, 1.0f));
      v_TexCoord = _e84.xy;
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

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const float3 a_Position = tint_symbol.a_Position;
  const float2 a_UV = tint_symbol.a_UV;
  const float4 a_Color = tint_symbol.a_Color;
  const float3 a_Normal = tint_symbol.a_Normal;
  const float a_PosMtxIdx = tint_symbol.a_PosMtxIdx;
  a_Position1 = a_Position;
  a_UV1 = a_UV;
  a_Color1 = a_Color;
  a_Normal1 = a_Normal;
  a_PosMtxIdx1 = a_PosMtxIdx;
  main1();
  const float4 _e11 = v_Color;
  const float2 _e13 = v_TexCoord;
  const float4 _e15 = gl_Position;
  const VertexOutput tint_symbol_3 = {_e11, _e13, _e15};
  const tint_symbol_2 tint_symbol_13 = {tint_symbol_3.v_Color, tint_symbol_3.v_TexCoord, tint_symbol_3.member};
  return tint_symbol_13;
}
