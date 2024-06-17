SKIP: FAILED

float4 main() {
  return float4(0.0f, 0.0f, 0.0f, 1.0f);
}

DXC validation failure:
hlsl.hlsl:1:1: error: Semantic must be defined for all outputs of an entry function or patch constant function
float4 main() {
^

