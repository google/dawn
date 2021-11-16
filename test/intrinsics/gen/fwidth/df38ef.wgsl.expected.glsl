#version 310 es
precision mediump float;

void fwidth_df38ef() {
  float res = fwidth(1.0f);
}

void fragment_main() {
  fwidth_df38ef();
  return;
}
void main() {
  fragment_main();
}


