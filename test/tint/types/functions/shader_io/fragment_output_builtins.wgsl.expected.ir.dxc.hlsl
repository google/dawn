SKIP: FAILED

float main1() {
  return 1.0f;
}

uint main2() {
  return 1u;
}

DXC validation failure:
hlsl.hlsl:1:1: error: Semantic must be defined for all outputs of an entry function or patch constant function
float main1() {
^

