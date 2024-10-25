struct main_outputs {
  int tint_symbol : SV_Target2;
};

struct main_inputs {
  nointerpolation int3 x : TEXCOORD1;
};


static bool continue_execution = true;
int f(int x) {
  if ((x == int(10))) {
    continue_execution = false;
  }
  return x;
}

int main_inner(int3 x) {
  int y = x[0u];
  {
    while(true) {
      int r = f(y);
      if ((r == int(0))) {
        break;
      }
      {
      }
      continue;
    }
  }
  return y;
}

main_outputs main(main_inputs inputs) {
  main_outputs v = {main_inner(inputs.x)};
  if (!(continue_execution)) {
    discard;
  }
  return v;
}

