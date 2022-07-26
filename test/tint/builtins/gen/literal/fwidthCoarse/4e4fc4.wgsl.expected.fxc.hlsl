void fwidthCoarse_4e4fc4() {
  float4 res = fwidth((1.0f).xxxx);
}

void fragment_main() {
  fwidthCoarse_4e4fc4();
  return;
}
