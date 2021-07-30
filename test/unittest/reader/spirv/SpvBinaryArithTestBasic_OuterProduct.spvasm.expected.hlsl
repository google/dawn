void main_1() {
  const float3 x_1 = (float3(50.0f, 60.0f, 70.0f) + float3(50.0f, 60.0f, 70.0f));
  const float2 x_2 = (float2(60.0f, 50.0f) + float2(50.0f, 60.0f));
  const float2x3 x_3 = float2x3(float3((x_2.x * x_1.x), (x_2.x * x_1.y), (x_2.x * x_1.z)), float3((x_2.y * x_1.x), (x_2.y * x_1.y), (x_2.y * x_1.z)));
  return;
}

void main() {
  main_1();
  return;
}
