#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
void foo(float x) {
  switch(int(x)) {
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
  switch(baz(int(x))) {
    default: {
      break;
    }
  }
}

void tint_symbol() {
}

