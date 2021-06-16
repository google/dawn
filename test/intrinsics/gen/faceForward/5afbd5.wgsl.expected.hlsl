void faceForward_5afbd5() {
  float3 res = faceforward(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  faceForward_5afbd5();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  faceForward_5afbd5();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  faceForward_5afbd5();
  return;
}
