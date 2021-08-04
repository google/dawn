groupshared int a;
groupshared int b;
groupshared int c;

void uses_a() {
  a = (a + 1);
}

void uses_b() {
  b = (b * 2);
}

void uses_a_and_b() {
  b = a;
}

void no_uses() {
}

void outer() {
  a = 0;
  uses_a();
  uses_a_and_b();
  uses_b();
  no_uses();
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void main1_inner(uint local_invocation_index) {
  {
    a = 0;
  }
  GroupMemoryBarrierWithGroupSync();
  a = 42;
  uses_a();
}

[numthreads(1, 1, 1)]
void main1(tint_symbol_1 tint_symbol) {
  main1_inner(tint_symbol.local_invocation_index);
  return;
}

struct tint_symbol_3 {
  uint local_invocation_index_1 : SV_GroupIndex;
};

void main2_inner(uint local_invocation_index_1) {
  {
    b = 0;
  }
  GroupMemoryBarrierWithGroupSync();
  b = 7;
  uses_b();
}

[numthreads(1, 1, 1)]
void main2(tint_symbol_3 tint_symbol_2) {
  main2_inner(tint_symbol_2.local_invocation_index_1);
  return;
}

struct tint_symbol_5 {
  uint local_invocation_index_2 : SV_GroupIndex;
};

void main3_inner(uint local_invocation_index_2) {
  {
    a = 0;
    b = 0;
  }
  GroupMemoryBarrierWithGroupSync();
  outer();
  no_uses();
}

[numthreads(1, 1, 1)]
void main3(tint_symbol_5 tint_symbol_4) {
  main3_inner(tint_symbol_4.local_invocation_index_2);
  return;
}

[numthreads(1, 1, 1)]
void main4() {
  no_uses();
  return;
}
