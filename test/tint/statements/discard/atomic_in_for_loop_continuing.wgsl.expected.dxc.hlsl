struct foo_outputs {
  int tint_symbol : SV_Target0;
};

struct foo_inputs {
  float tint_member : TEXCOORD0;
  float2 coord : TEXCOORD1;
};


Texture2D<float4> t : register(t0);
SamplerState s : register(s1);
RWByteAddressBuffer a : register(u2);
int tint_f32_to_i32(float value) {
  return int(clamp(value, -2147483648.0f, 2147483520.0f));
}

int foo_inner(float v, float2 coord) {
  if ((v == 0.0f)) {
    discard;
  }
  int result = tint_f32_to_i32(t.Sample(s, coord).x);
  {
    uint2 tint_loop_idx = (4294967295u).xx;
    int i = int(0);
    while(true) {
      if (all((tint_loop_idx == (0u).xx))) {
        break;
      }
      if ((i < int(10))) {
      } else {
        break;
      }
      result = (result + i);
      {
        uint tint_low_inc = (tint_loop_idx.x - 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 4294967295u));
        tint_loop_idx.y = (tint_loop_idx.y - tint_carry);
        int v_1 = int(0);
        a.InterlockedAdd(int(0u), int(1), v_1);
        i = v_1;
      }
      continue;
    }
  }
  return result;
}

foo_outputs foo(foo_inputs inputs) {
  foo_outputs v_2 = {foo_inner(inputs.tint_member, inputs.coord)};
  return v_2;
}

