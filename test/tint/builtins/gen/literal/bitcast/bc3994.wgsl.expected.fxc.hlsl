SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0, space2);

void bitcast_bc3994() {
  vector<float16_t, 4> res = vector<float16_t, 4>(float16_t(0.00000005960464477539h), float16_t(0.0h), float16_t(0.00000005960464477539h), float16_t(0.0h));
  prevent_dce.Store<vector<float16_t, 4> >(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  bitcast_bc3994();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  bitcast_bc3994();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  bitcast_bc3994();
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000127B6F41BB0(4,10-18): error X3000: syntax error: unexpected token 'float16_t'
C:\src\dawn\Shader@0x00000127B6F41BB0(5,3-19): error X3018: invalid subscript 'Store'

