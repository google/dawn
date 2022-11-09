static bool tint_discarded = false;
Texture2D<float4> t : register(t0, space0);
SamplerState s : register(s1, space0);
RWByteAddressBuffer a : register(u2, space0);

struct tint_symbol_2 {
  float tint_symbol : TEXCOORD0;
  float2 coord : TEXCOORD1;
};
struct tint_symbol_3 {
  int value : SV_Target0;
};

int tint_atomicAdd(RWByteAddressBuffer buffer, uint offset, int value) {
  int original_value = 0;
  buffer.InterlockedAdd(offset, value, original_value);
  return original_value;
}


int foo_inner(float tint_symbol, float2 coord) {
  if ((tint_symbol == 0.0f)) {
    tint_discarded = true;
  }
  int result = int(t.Sample(s, coord).x);
  {
    int i = 0;
    while (true) {
      if (!((i < 10))) {
        break;
      }
      {
        result = (result + i);
      }
      {
        int tint_symbol_4 = 0;
        if (!(tint_discarded)) {
          tint_symbol_4 = tint_atomicAdd(a, 0u, 1);
        }
        i = tint_symbol_4;
      }
    }
  }
  return result;
}

tint_symbol_3 foo(tint_symbol_2 tint_symbol_1) {
  const int inner_result = foo_inner(tint_symbol_1.tint_symbol, tint_symbol_1.coord);
  tint_symbol_3 wrapper_result = (tint_symbol_3)0;
  wrapper_result.value = inner_result;
  if (tint_discarded) {
    discard;
  }
  return wrapper_result;
}
