struct foo_outputs {
  int tint_symbol_1 : SV_Target0;
};

struct foo_inputs {
  float tint_symbol : TEXCOORD0;
  float2 coord : TEXCOORD1;
};


Texture2D<float4> t : register(t0);
SamplerState s : register(s1);
RWByteAddressBuffer a : register(u2);
int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : (int(-2147483648)))) : (int(2147483647)));
}

int foo_inner(float tint_symbol, float2 coord) {
  if ((tint_symbol == 0.0f)) {
    discard;
  }
  int result = tint_f32_to_i32(t.Sample(s, coord).x);
  {
    uint2 tint_loop_idx = (0u).xx;
    int i = int(0);
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      if ((i < int(10))) {
      } else {
        break;
      }
      result = (result + i);
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
        int v = int(0);
        a.InterlockedAdd(int(0u), int(1), v);
        i = v;
      }
      continue;
    }
  }
  return result;
}

foo_outputs foo(foo_inputs inputs) {
  foo_outputs v_1 = {foo_inner(inputs.tint_symbol, inputs.coord)};
  return v_1;
}

