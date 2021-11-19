#version 310 es
precision mediump float;

shared int a;

void uses_a() {
  a = (a + 1);
}

struct tint_symbol_1 {
  uint local_invocation_index;
};

void main1_inner(uint local_invocation_index) {
  {
    a = 0;
  }
  memoryBarrierShared();
  a = 42;
  uses_a();
}

struct tint_symbol_3 {
  uint local_invocation_index_1;
};
struct tint_symbol_5 {
  uint local_invocation_index_2;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main1(tint_symbol_1 tint_symbol) {
  main1_inner(tint_symbol.local_invocation_index);
  return;
}
void main() {
  tint_symbol_1 inputs;
  inputs.local_invocation_index = uint(gl_LocalInvocationIndex);
  main1(inputs);
}


#version 310 es
precision mediump float;

shared int b;

void uses_b() {
  b = (b * 2);
}

struct tint_symbol_1 {
  uint local_invocation_index;
};
struct tint_symbol_3 {
  uint local_invocation_index_1;
};

void main2_inner(uint local_invocation_index_1) {
  {
    b = 0;
  }
  memoryBarrierShared();
  b = 7;
  uses_b();
}

struct tint_symbol_5 {
  uint local_invocation_index_2;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main2(tint_symbol_3 tint_symbol_2) {
  main2_inner(tint_symbol_2.local_invocation_index_1);
  return;
}
void main() {
  tint_symbol_3 inputs;
  inputs.local_invocation_index_1 = uint(gl_LocalInvocationIndex);
  main2(inputs);
}


#version 310 es
precision mediump float;

shared int a;
shared int b;

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
  uint local_invocation_index;
};
struct tint_symbol_3 {
  uint local_invocation_index_1;
};
struct tint_symbol_5 {
  uint local_invocation_index_2;
};

void main3_inner(uint local_invocation_index_2) {
  {
    a = 0;
    b = 0;
  }
  memoryBarrierShared();
  outer();
  no_uses();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main3(tint_symbol_5 tint_symbol_4) {
  main3_inner(tint_symbol_4.local_invocation_index_2);
  return;
}
void main() {
  tint_symbol_5 inputs;
  inputs.local_invocation_index_2 = uint(gl_LocalInvocationIndex);
  main3(inputs);
}


#version 310 es
precision mediump float;

void no_uses() {
}

struct tint_symbol_1 {
  uint local_invocation_index;
};
struct tint_symbol_3 {
  uint local_invocation_index_1;
};
struct tint_symbol_5 {
  uint local_invocation_index_2;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main4() {
  no_uses();
  return;
}
void main() {
  main4();
}


