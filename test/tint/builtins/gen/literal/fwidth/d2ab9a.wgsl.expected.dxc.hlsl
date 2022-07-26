void fwidth_d2ab9a() {
  float4 res = fwidth((1.0f).xxxx);
}

void fragment_main() {
  fwidth_d2ab9a();
  return;
}
