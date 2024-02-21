SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0, space2);

void bitcast_436211() {
  float16_t arg_0 = float16_t(1.0h);
  float16_t res = arg_0;
  prevent_dce.Store<float16_t>(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  bitcast_436211();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  bitcast_436211();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  bitcast_436211();
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002296BCE02B0(4,3-11): error X3000: unrecognized identifier 'float16_t'
C:\src\dawn\Shader@0x000002296BCE02B0(4,13-17): error X3000: unrecognized identifier 'arg_0'

