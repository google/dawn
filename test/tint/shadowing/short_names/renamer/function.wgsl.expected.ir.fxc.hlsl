struct main_outputs {
  float4 tint_symbol : SV_Position;
};

struct main_inputs {
  uint VertexIndex : SV_VertexID;
};


int vec4f() {
  return int(0);
}

float vec2f(int i) {
  return float(i);
}

bool vec2i(float f) {
  return bool(f);
}

float4 main_inner(uint VertexIndex) {
  return ((vec2i(vec2f(vec4f()))) ? ((1.0f).xxxx) : ((0.0f).xxxx));
}

main_outputs main(main_inputs inputs) {
  main_outputs v = {main_inner(inputs.VertexIndex)};
  return v;
}

