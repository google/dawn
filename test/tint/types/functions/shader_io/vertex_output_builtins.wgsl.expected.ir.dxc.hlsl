SKIP: FAILED

float4 main() {
  return float4(1.0f, 2.0f, 3.0f, 4.0f);
}

DXC validation failure:
hlsl.hlsl:1:1: error: Semantic must be defined for all outputs of an entry function or patch constant function
float4 main() {
^

