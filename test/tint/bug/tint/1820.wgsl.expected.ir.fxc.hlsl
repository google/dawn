
static int global = int(0);
int tint_f32_to_i32(float value) {
  return int(clamp(value, -2147483648.0f, 2147483520.0f));
}

void foo(float x) {
  tint_f32_to_i32(x);
  {
    while(true) {
      break;
    }
  }
}

int baz(int x) {
  global = int(42);
  return x;
}

void bar(float x) {
  baz(tint_f32_to_i32(x));
  {
    while(true) {
      break;
    }
  }
}

void main() {
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

