void dpdyFine_1fb7ab() {
  float3 res = ddy_fine((1.0f).xxx);
}

void fragment_main() {
  dpdyFine_1fb7ab();
  return;
}
