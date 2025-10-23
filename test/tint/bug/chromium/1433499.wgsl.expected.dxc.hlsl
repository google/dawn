
void f(inout float p) {
}

[numthreads(1, 1, 1)]
void main() {
  float a = 1.0f;
  f(a);
}

