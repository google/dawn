void fwidthFine_523fdc() {
  float3 res = fwidth((1.0f).xxx);
}

void fragment_main() {
  fwidthFine_523fdc();
  return;
}
