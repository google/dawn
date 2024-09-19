struct vec4f {
  int i;
};

struct main_outputs {
  float4 tint_symbol : SV_Position;
};

struct main_inputs {
  uint VertexIndex : SV_VertexID;
};


float4 main_inner(uint VertexIndex) {
  vec4f v = {int(1)};
  vec4f s = v;
  float f = float(s.i);
  bool b = bool(f);
  return ((b) ? ((1.0f).xxxx) : ((0.0f).xxxx));
}

main_outputs main(main_inputs inputs) {
  main_outputs v_1 = {main_inner(inputs.VertexIndex)};
  return v_1;
}

