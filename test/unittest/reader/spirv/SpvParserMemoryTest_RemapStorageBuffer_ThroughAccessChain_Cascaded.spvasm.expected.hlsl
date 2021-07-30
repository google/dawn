RWByteAddressBuffer myvar : register(u0, space0);

void main_1() {
  myvar.Store(8u, asuint(0u));
  return;
}

void main() {
  main_1();
  return;
}
