#version 310 es
precision mediump float;


layout (binding = 0) buffer SB_1 {
  int data[];
} tint_symbol;

struct tint_symbol_3 {
  uvec3 id;
};

void tint_symbol_1_inner(uvec3 id) {
  tint_symbol.data[id.x] = (tint_symbol.data[id.x] + 1);
}

layout(local_size_x = 1, local_size_y = 2, local_size_z = 3) in;
void tint_symbol_1(tint_symbol_3 tint_symbol_2) {
  tint_symbol_1_inner(tint_symbol_2.id);
  return;
}
void main() {
  tint_symbol_3 inputs;
  inputs.id = gl_GlobalInvocationID;
  tint_symbol_1(inputs);
}


