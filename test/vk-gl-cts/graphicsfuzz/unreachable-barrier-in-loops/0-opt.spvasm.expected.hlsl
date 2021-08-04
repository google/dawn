static uint3 gl_GlobalInvocationID = uint3(0u, 0u, 0u);
cbuffer cbuffer_x_10 : register(b1, space0) {
  uint4 x_10[1];
};
cbuffer cbuffer_x_13 : register(b2, space0) {
  uint4 x_13[1];
};
RWByteAddressBuffer x_15 : register(u0, space0);

void main_1() {
  float A[1] = (float[1])0;
  int i = 0;
  float4 value = float4(0.0f, 0.0f, 0.0f, 0.0f);
  int m = 0;
  int l = 0;
  int n = 0;
  A[0] = 0.0f;
  i = 0;
  {
    for(; (i < 50); i = (i + 1)) {
      if ((i > 0)) {
        const float x_68 = A[0];
        const float x_70 = A[0];
        A[0] = (x_70 + x_68);
      }
    }
  }
  while (true) {
    const uint x_80 = gl_GlobalInvocationID.x;
    if ((x_80 < 100u)) {
      value = float4(0.0f, 0.0f, 0.0f, 1.0f);
      m = 0;
      {
        for(; (m < 1); m = (m + 1)) {
          l = 0;
          {
            for(; (l < 1); l = (l + 1)) {
              const float x_100 = asfloat(x_10[0].x);
              const float x_102 = asfloat(x_10[0].y);
              if ((x_100 > x_102)) {
                return;
              }
            }
          }
        }
      }
      n = 0;
      {
        for(; (n < 1); n = (n + 1)) {
          const float x_118 = asfloat(x_10[0].x);
          const float x_120 = asfloat(x_10[0].y);
          if ((x_118 > x_120)) {
            GroupMemoryBarrierWithGroupSync();
          }
        }
      }
    } else {
      const uint x_127 = gl_GlobalInvocationID.x;
      if ((x_127 < 120u)) {
        const float x_133 = A[0];
        const float x_135 = asfloat(x_13[0].x);
        const float x_138 = A[0];
        const float x_140 = asfloat(x_13[0].y);
        value = float4((x_133 / x_135), (x_138 / x_140), 0.0f, 1.0f);
      } else {
        const float x_144 = asfloat(x_10[0].x);
        const float x_146 = asfloat(x_10[0].y);
        if ((x_144 > x_146)) {
          {
            if (false) {
            } else {
              break;
            }
          }
          continue;
        }
      }
    }
    {
      if (false) {
      } else {
        break;
      }
    }
  }
  const float x_151 = value.x;
  x_15.Store((4u * uint(0)), asuint(int(x_151)));
  const float x_155 = value.y;
  x_15.Store(4u, asuint(int(x_155)));
  const float x_159 = value.z;
  x_15.Store(8u, asuint(int(x_159)));
  const float x_163 = value.w;
  x_15.Store(12u, asuint(int(x_163)));
  return;
}

struct tint_symbol_1 {
  uint3 gl_GlobalInvocationID_param : SV_DispatchThreadID;
};

void main_inner(uint3 gl_GlobalInvocationID_param) {
  gl_GlobalInvocationID = gl_GlobalInvocationID_param;
  main_1();
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.gl_GlobalInvocationID_param);
  return;
}
