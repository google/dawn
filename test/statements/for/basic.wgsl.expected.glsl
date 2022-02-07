#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
void some_loop_body() {
}

void f() {
  {
    for(int i = 0; (i < 5); i = (i + 1)) {
      some_loop_body();
    }
  }
}

