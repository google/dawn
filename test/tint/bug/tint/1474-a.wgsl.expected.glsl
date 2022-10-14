#version 310 es

void tint_symbol() {
  while(true) {
    if (true) {
      break;
    } else {
      return;
    }
  }
  int x = 5;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
