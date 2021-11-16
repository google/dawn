#version 310 es
precision mediump float;

struct tint_symbol {
  int value;
};

int main0_inner() {
  return 1;
}

struct tint_symbol_1 {
  uint value;
};
struct tint_symbol_2 {
  float value;
};
struct tint_symbol_3 {
  vec4 value;
};

tint_symbol main0() {
  int inner_result = main0_inner();
  tint_symbol wrapper_result = tint_symbol(0);
  wrapper_result.value = inner_result;
  return wrapper_result;
}
out int value;
void main() {
  tint_symbol outputs;
  outputs = main0();
  value = outputs.value;
}


#version 310 es
precision mediump float;

struct tint_symbol {
  int value;
};
struct tint_symbol_1 {
  uint value;
};

uint main1_inner() {
  return 1u;
}

struct tint_symbol_2 {
  float value;
};
struct tint_symbol_3 {
  vec4 value;
};

tint_symbol_1 main1() {
  uint inner_result_1 = main1_inner();
  tint_symbol_1 wrapper_result_1 = tint_symbol_1(0u);
  wrapper_result_1.value = inner_result_1;
  return wrapper_result_1;
}
out uint value;
void main() {
  tint_symbol_1 outputs;
  outputs = main1();
  value = outputs.value;
}


#version 310 es
precision mediump float;

struct tint_symbol {
  int value;
};
struct tint_symbol_1 {
  uint value;
};
struct tint_symbol_2 {
  float value;
};

float main2_inner() {
  return 1.0f;
}

struct tint_symbol_3 {
  vec4 value;
};

tint_symbol_2 main2() {
  float inner_result_2 = main2_inner();
  tint_symbol_2 wrapper_result_2 = tint_symbol_2(0.0f);
  wrapper_result_2.value = inner_result_2;
  return wrapper_result_2;
}
out float value;
void main() {
  tint_symbol_2 outputs;
  outputs = main2();
  value = outputs.value;
}


#version 310 es
precision mediump float;

struct tint_symbol {
  int value;
};
struct tint_symbol_1 {
  uint value;
};
struct tint_symbol_2 {
  float value;
};
struct tint_symbol_3 {
  vec4 value;
};

vec4 main3_inner() {
  return vec4(1.0f, 2.0f, 3.0f, 4.0f);
}

tint_symbol_3 main3() {
  vec4 inner_result_3 = main3_inner();
  tint_symbol_3 wrapper_result_3 = tint_symbol_3(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result_3.value = inner_result_3;
  return wrapper_result_3;
}
out vec4 value;
void main() {
  tint_symbol_3 outputs;
  outputs = main3();
  value = outputs.value;
}


