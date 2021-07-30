RWByteAddressBuffer myvar : register(u0, space0);

void main_1() {
  myvar.Store(12u, asuint(42.0f));
  return;
}

void main() {
  main_1();
  return;
}
