#version 310 es


struct S {
  vec2 field0;
  uint field1;
};

struct S_1 {
  uint field0;
};

struct S_2 {
  S_1 field0;
};

shared S x_28[4096];
shared uint x_34;
shared uint x_35;
shared uint x_36;
shared uint x_37;
uvec3 x_3 = uvec3(0u);
layout(binding = 1, std140)
uniform tint_symbol_2_1_ubo {
  S_2 tint_symbol_1;
} v;
layout(binding = 2, std430)
buffer S_3_1_ssbo {
  vec4 field0[];
} x_9;
layout(binding = 3, std430)
buffer S_4_1_ssbo {
  vec4 field0[];
} x_12;
void main_1() {
  uint x_54 = 0u;
  uint x_58 = 0u;
  vec4 x_85 = vec4(0.0f);
  uint x_88 = 0u;
  uint x_52 = x_3.x;
  x_54 = 0u;
  {
    while(true) {
      uint x_55 = 0u;
      x_58 = v.tint_symbol_1.field0.field0;
      if ((x_54 < x_58)) {
      } else {
        break;
      }
      uint x_62 = (x_54 + x_52);
      if ((x_62 >= x_58)) {
        vec4 x_67 = x_9.field0[x_62];
        x_28[x_62] = S(((x_67.xy + x_67.zw) * 0.5f), x_62);
      }
      {
        x_55 = (x_54 + 32u);
        x_54 = x_55;
      }
      continue;
    }
  }
  barrier();
  int x_74 = int(x_58);
  vec2 x_76 = x_28[0].field0;
  if ((x_52 == 0u)) {
    uvec2 x_80 = floatBitsToUint(x_76);
    uint x_81 = x_80[0u];
    atomicExchange(x_34, x_81);
    uint x_82 = x_80[1u];
    atomicExchange(x_35, x_82);
    atomicExchange(x_36, x_81);
    atomicExchange(x_37, x_82);
  }
  x_85 = x_76.xyxy;
  x_88 = 1u;
  {
    while(true) {
      vec4 x_111 = vec4(0.0f);
      vec4 x_86 = vec4(0.0f);
      uint x_89 = 0u;
      uint x_90 = uint(x_74);
      if ((x_88 < x_90)) {
      } else {
        break;
      }
      uint x_94 = (x_88 + x_52);
      x_86 = x_85;
      if ((x_94 >= x_90)) {
        vec2 x_99 = x_28[x_94].field0;
        vec2 x_101 = min(x_85.xy, x_99);
        vec4 x_103_1 = x_85;
        x_103_1[0u] = x_101[0u];
        vec4 x_103 = x_103_1;
        vec4 x_105_1 = x_103;
        x_105_1[1u] = x_101[1u];
        vec4 x_105 = x_105_1;
        vec2 x_107 = max(x_105_1.zw, x_99);
        vec4 x_109_1 = x_105;
        x_109_1[2u] = x_107[0u];
        x_111 = x_109_1;
        x_111[3u] = x_107[1u];
        x_86 = x_111;
      }
      {
        x_89 = (x_88 + 32u);
        x_85 = x_86;
        x_88 = x_89;
      }
      continue;
    }
  }
  barrier();
  uint x_114 = atomicMin(x_34, floatBitsToUint(x_85.x));
  uint x_117 = atomicMin(x_35, floatBitsToUint(x_85.y));
  uint x_120 = atomicMax(x_36, floatBitsToUint(x_85.z));
  uint x_123 = atomicMax(x_37, floatBitsToUint(x_85.w));
  barrier();
  float v_1 = uintBitsToFloat(atomicOr(x_34, 0u));
  float v_2 = uintBitsToFloat(atomicOr(x_35, 0u));
  float v_3 = uintBitsToFloat(atomicOr(x_36, 0u));
  x_12.field0[0] = vec4(v_1, v_2, v_3, uintBitsToFloat(atomicOr(x_37, 0u)));
}
void tint_symbol_inner(uvec3 x_3_param, uint tint_local_index) {
  if ((tint_local_index == 0u)) {
    atomicExchange(x_34, 0u);
    atomicExchange(x_35, 0u);
    atomicExchange(x_36, 0u);
    atomicExchange(x_37, 0u);
  }
  {
    uint v_4 = 0u;
    v_4 = tint_local_index;
    while(true) {
      uint v_5 = v_4;
      if ((v_5 >= 4096u)) {
        break;
      }
      x_28[v_5] = S(vec2(0.0f), 0u);
      {
        v_4 = (v_5 + 32u);
      }
      continue;
    }
  }
  barrier();
  x_3 = x_3_param;
  main_1();
}
layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_inner(gl_LocalInvocationID, gl_LocalInvocationIndex);
}
