SKIP: FAILED

float b() {
  return 2.29999995231628417969f;
}

int c() {
  return 1;
}

void a() {
  float a = b(c(2u));
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

DXC validation failure:
hlsl.hlsl:10:15: error: no matching function for call to 'c'
  float a = b(c(2u));
              ^
hlsl.hlsl:5:5: note: candidate function not viable: requires 0 arguments, but 1 was provided
int c() {
    ^

