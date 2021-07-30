static uint4 myvar = uint4(0u, 0u, 0u, 0u);

void main_1() {
  myvar.z = 42u;
  return;
}

void main() {
  main_1();
  return;
}
