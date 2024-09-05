#version 310 es
precision highp float;
precision highp int;

void main() {
  {
    while(true) {
      if (false) {
      } else {
        break;
      }
      {
        if (false) { break; }
      }
      continue;
    }
  }
}
