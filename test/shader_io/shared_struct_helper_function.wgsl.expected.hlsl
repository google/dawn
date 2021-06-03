struct VertexOutput {
  float4 pos;
  int loc0;
};

VertexOutput foo(float x) {
  const VertexOutput tint_symbol = {float4(x, x, x, 1.0f), 42};
  return tint_symbol;
}

VertexOutput vert_main1() {
  return foo(0.5f);
}

VertexOutput vert_main2() {
  return foo(0.25f);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

