SKIP: FAILED

float4 main() {
  return float4(0.10000000149011611938f, 0.20000000298023223877f, 0.30000001192092895508f, 0.40000000596046447754f);
}

DXC validation failure:
hlsl.hlsl:1:1: error: Semantic must be defined for all outputs of an entry function or patch constant function
float4 main() {
^

