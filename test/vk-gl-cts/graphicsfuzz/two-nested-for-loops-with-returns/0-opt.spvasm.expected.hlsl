RWByteAddressBuffer x_9 : register(u0, space0);

float nb_mod_() {
  float s = 0.0f;
  int i = 0;
  int GLF_live1i = 0;
  int GLF_live1_looplimiter2 = 0;
  float x_51 = 0.0f;
  float x_56_phi = 0.0f;
  s = 0.0f;
  i = 5;
  while (true) {
    float x_50 = 0.0f;
    float x_51_phi = 0.0f;
    x_56_phi = 0.0f;
    if ((5 < 800)) {
    } else {
      break;
    }
    GLF_live1i = 0;
    while (true) {
      x_51_phi = 0.0f;
      if ((0 < 20)) {
      } else {
        break;
      }
      if ((0 >= 5)) {
        x_50 = (0.0f + 1.0f);
        s = x_50;
        x_51_phi = x_50;
        break;
      }
      return 42.0f;
    }
    x_51 = x_51_phi;
    if ((float(5) <= x_51)) {
      x_56_phi = x_51;
      break;
    }
    return 42.0f;
  }
  return x_56_phi;
}

void main_1() {
  const float x_32 = nb_mod_();
  x_9.Store((4u * uint(0)), asuint(x_32));
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
