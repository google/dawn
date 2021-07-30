void set_uint4(inout uint4 vec, int idx, uint val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

static uint4 myvar = uint4(0u, 0u, 0u, 0u);
static uint4 x_10 = uint4(0u, 0u, 0u, 0u);

void main_1() {
  const uint a_dynamic_index = x_10.z;
  set_uint4(myvar, a_dynamic_index, 42u);
  return;
}

void main() {
  main_1();
  return;
}
