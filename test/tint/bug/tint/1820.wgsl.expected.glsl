#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
int tint_ftoi(float v) {
  return mix(2147483647, mix(int(v), (-2147483647 - 1), (v < -2147483648.0f)), (v <= 2147483520.0f));
}

void foo(float x) {
  switch(tint_ftoi(x)) {
    default: {
      break;
    }
  }
}

int global = 0;
int baz(int x) {
  global = 42;
  return x;
}

void bar(float x) {
  switch(baz(tint_ftoi(x))) {
    default: {
      break;
    }
  }
}

void tint_symbol() {
}

