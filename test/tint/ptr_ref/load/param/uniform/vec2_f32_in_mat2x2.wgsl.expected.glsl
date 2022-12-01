#version 310 es

layout(binding = 0, std140) uniform S_block_std140_ubo {
  vec2 inner_0;
  vec2 inner_1;
} S;

vec2 load_S_inner_p0(uint p0) {
  switch(p0) {
    case 0u: {
      return S.inner_0;
      break;
    }
    case 1u: {
      return S.inner_1;
      break;
    }
    default: {
      return vec2(0.0f);
      break;
    }
  }
}

vec2 func_S_X(uint pointer[1]) {
  return load_S_inner_p0(uint(pointer[0]));
}

void tint_symbol() {
  uint tint_symbol_1[1] = uint[1](1u);
  vec2 r = func_S_X(tint_symbol_1);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
