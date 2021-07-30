static float3x4 myvar = float3x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  myvar[2u] = float4(42.0f, 42.0f, 42.0f, 42.0f);
  return;
}

void main() {
  main_1();
  return;
}
