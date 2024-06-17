SKIP: FAILED

int main0() {
  return 1;
}

uint main1() {
  return 1u;
}

float main2() {
  return 1.0f;
}

float4 main3() {
  return float4(1.0f, 2.0f, 3.0f, 4.0f);
}

DXC validation failure:
hlsl.hlsl:1:1: error: Semantic must be defined for all outputs of an entry function or patch constant function
int main0() {
^

