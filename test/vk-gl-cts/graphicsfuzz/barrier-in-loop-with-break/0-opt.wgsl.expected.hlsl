static uint3 gl_LocalInvocationID = uint3(0u, 0u, 0u);
RWByteAddressBuffer x_7 : register(u0, space0);
cbuffer cbuffer_x_10 : register(b1, space0) {
  uint4 x_10[1];
};

void main_1() {
  int lid = 0;
  int val = 0;
  int i = 0;
  const uint x_40 = gl_LocalInvocationID.x;
  lid = asint(x_40);
  const int x_43 = asint(x_7.Load(0u));
  val = x_43;
  i = 0;
  {
    for(; (i < 2); i = (i + 1)) {
      if ((lid > 0)) {
        const int x_58 = asint(x_7.Load((4u + (4u * uint((lid - 1))))));
        val = (val + x_58);
        const float x_62 = asfloat(x_10[0].x);
        if ((x_62 > 100.0f)) {
          break;
        }
      }
      GroupMemoryBarrierWithGroupSync();
    }
  }
  if ((lid == 0)) {
    x_7.Store((4u + (4u * uint(0))), asuint(42));
  }
  return;
}

struct tint_symbol_1 {
  uint3 gl_LocalInvocationID_param : SV_GroupThreadID;
};

void main_inner(uint3 gl_LocalInvocationID_param) {
  gl_LocalInvocationID = gl_LocalInvocationID_param;
  main_1();
}

[numthreads(16, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.gl_LocalInvocationID_param);
  return;
}
