SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0, space2);

void bitcast_3e7b47() {
  vector<float16_t, 4> arg_0 = (float16_t(1.0h)).xxxx;
  vector<float16_t, 4> res = arg_0;
  prevent_dce.Store<vector<float16_t, 4> >(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  bitcast_3e7b47();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  bitcast_3e7b47();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  bitcast_3e7b47();
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002727ADE51D0(4,10-18): error X3000: syntax error: unexpected token 'float16_t'
C:\src\dawn\Shader@0x000002727ADE51D0(5,10-18): error X3000: syntax error: unexpected token 'float16_t'
C:\src\dawn\Shader@0x000002727ADE51D0(6,3-19): error X3018: invalid subscript 'Store'

