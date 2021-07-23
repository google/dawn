RWByteAddressBuffer x_4 : register(u0, space0);
cbuffer cbuffer_x_6 : register(b1, space0) {
  uint4 x_6[1];
};

void main_1() {
  int i = 0;
  float GLF_live3s = 0.0f;
  int i_1 = 0;
  int z = 0;
  x_4.Store(0u, asuint(42));
  const float x_37 = asfloat(x_6[0].x);
  const float x_39 = asfloat(x_6[0].y);
  if ((x_37 > x_39)) {
    GroupMemoryBarrierWithGroupSync();
  }
  const float x_44 = asfloat(x_6[0].x);
  const float x_46 = asfloat(x_6[0].y);
  if ((x_44 > x_46)) {
    GroupMemoryBarrierWithGroupSync();
  }
  const float x_51 = asfloat(x_6[0].y);
  i = int(x_51);
  {
    for(; (i > 0); i = (i - 1)) {
      GroupMemoryBarrierWithGroupSync();
    }
  }
  GLF_live3s = 0.0f;
  while (true) {
    i_1 = 1;
    {
      for(; (i_1 < 2); i_1 = (i_1 + 1)) {
        const float x_74 = asfloat(x_6[0].x);
        if ((x_74 > 1.0f)) {
          GroupMemoryBarrierWithGroupSync();
        }
        const float x_79 = asfloat(x_6[0].x);
        const float x_81 = asfloat(x_6[0].y);
        if ((x_79 > x_81)) {
          GroupMemoryBarrierWithGroupSync();
        }
        const float x_86 = asfloat(x_6[0].y);
        z = int(x_86);
        {
          for(; (z > 0); z = (z - 1)) {
            GLF_live3s = (GLF_live3s + 1.0f);
          }
        }
        if ((i_1 >= 1)) {
          const float x_104 = asfloat(x_6[0].x);
          if ((x_104 > 1.0f)) {
            GroupMemoryBarrierWithGroupSync();
          }
        }
      }
    }
    {
      const float x_111 = asfloat(x_6[0].x);
      const float x_113 = asfloat(x_6[0].y);
      if ((x_111 > x_113)) {
      } else {
        break;
      }
    }
  }
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
