static float4 GLF_live2gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_9 : register(b1, space0) {
  uint4 x_9[1];
};
RWByteAddressBuffer x_12 : register(u0, space0);

void main_1() {
  int GLF_live2_looplimiter1 = 0;
  int i = 0;
  int j = 0;
  float GLF_dead3x = 0.0f;
  float x_51 = 0.0f;
  int GLF_dead3k = 0;
  GLF_live2_looplimiter1 = 0;
  i = 0;
  {
    for(; (i < 1); i = (i + 1)) {
      if ((GLF_live2_looplimiter1 >= 3)) {
        j = 0;
        {
          for(; (j < 1); j = (j + 1)) {
            const float x_13 = GLF_live2gl_FragCoord.x;
            if ((int(x_13) < 120)) {
            } else {
              GroupMemoryBarrierWithGroupSync();
            }
          }
        }
        break;
      }
    }
  }
  const float x_81 = asfloat(x_9[0].x);
  const float x_83 = asfloat(x_9[0].y);
  if ((x_81 > x_83)) {
    const float x_14 = GLF_live2gl_FragCoord.x;
    x_51 = x_14;
  } else {
    x_51 = 0.0f;
  }
  GLF_dead3x = x_51;
  GLF_dead3k = 0;
  {
    for(; (GLF_dead3k < 2); GLF_dead3k = (GLF_dead3k + 1)) {
      if ((GLF_dead3x > 4.0f)) {
        break;
      }
      const float x_16 = GLF_live2gl_FragCoord.x;
      GLF_dead3x = x_16;
      GroupMemoryBarrierWithGroupSync();
    }
  }
  x_12.Store((4u * uint(0)), asuint(42u));
  return;
}

[numthreads(1, 18, 6)]
void main() {
  main_1();
  return;
}
