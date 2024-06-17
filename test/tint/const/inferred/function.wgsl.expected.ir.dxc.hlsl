SKIP: FAILED

void const_decls() {
}

float4 main() {
  return (0.0f).xxxx;
}

DXC validation failure:
hlsl.hlsl:4:1: error: Semantic must be defined for all outputs of an entry function or patch constant function
float4 main() {
^

