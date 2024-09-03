#version 310 es
precision highp float;
precision highp int;


int non_uniform_global;
float tint_symbol;
bool continue_execution = true;
void main() {
  if ((non_uniform_global < 0)) {
    continue_execution = false;
  }
  float v = dFdx(1.0f);
  if (continue_execution) {
    tint_symbol = v;
  }
  if ((tint_symbol < 0.0f)) {
    int i = 0;
    {
      while(true) {
        float v_1 = tint_symbol;
        if ((v_1 > float(i))) {
          float v_2 = float(i);
          if (continue_execution) {
            tint_symbol = v_2;
          }
          if (!(continue_execution)) {
            discard;
          }
          return;
        }
        {
          i = (i + 1);
          if ((i == 5)) { break; }
        }
        continue;
      }
    }
    if (!(continue_execution)) {
      discard;
    }
    return;
  }
  if (!(continue_execution)) {
    discard;
  }
}
