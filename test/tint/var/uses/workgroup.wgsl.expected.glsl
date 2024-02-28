#version 310 es

shared int a;
void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    a = 0;
  }
  barrier();
}

void uses_a() {
  a = (a + 1);
}

void main1(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  a = 42;
  uses_a();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main1(gl_LocalInvocationIndex);
  return;
}
#version 310 es

shared int b;
void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    b = 0;
  }
  barrier();
}

void uses_b() {
  b = (b * 2);
}

void main2(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  b = 7;
  uses_b();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main2(gl_LocalInvocationIndex);
  return;
}
#version 310 es

shared int a;
shared int b;
void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    a = 0;
    b = 0;
  }
  barrier();
}

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

void main3(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  outer();
  no_uses();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main3(gl_LocalInvocationIndex);
  return;
}
#version 310 es

void no_uses() {
}

void main4() {
  no_uses();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main4();
  return;
}
