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

