#version 310 es
precision highp float;
precision highp int;


int tint_symbol[4];
void foo() {
  {
    int i = 0;
    while(true) {
      if ((i < 4)) {
      } else {
        break;
      }
      switch(tint_symbol[i]) {
        case 1:
        {
          {
            i = (i + 1);
          }
          continue;
        }
        default:
        {
          tint_symbol[i] = 2;
          break;
        }
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
}
void main() {
  foo();
}
