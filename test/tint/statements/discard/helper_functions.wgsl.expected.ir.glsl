#version 310 es
precision highp float;
precision highp int;


int non_uniform_global;
float tint_symbol;
bool continue_execution = true;
void foo() {
  if ((non_uniform_global < 0)) {
    continue_execution = false;
  }
}
void bar() {
  float v = dFdx(1.0f);
  if (continue_execution) {
    tint_symbol = v;
  }
}
void main() {
  foo();
  bar();
  if (!(continue_execution)) {
    discard;
  }
}
