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
static bool continue_execution = true;
int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : (int(-2147483648)))) : (int(2147483647)));
}

int foo_inner(float tint_symbol, float2 coord) {
  if ((tint_symbol == 0.0f)) {
    continue_execution = false;
  }
  int result = tint_f32_to_i32(t.Sample(s, coord)[0u]);
  {
    int i = int(0);
    while(true) {
      if ((i < int(10))) {
      } else {
        break;
      }
      result = (result + i);
      {
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
  if (!(continue_execution)) {
    discard;
  }
  foo_outputs v_2 = v_1;
  return v_2;
}

