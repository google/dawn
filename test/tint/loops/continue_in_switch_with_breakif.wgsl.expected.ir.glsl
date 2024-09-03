#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int i = 0;
  {
    while(true) {
      switch(i) {
        case 0:
        {
          {
            i = (i + 1);
            if ((i >= 4)) { break; }
          }
          continue;
        }
        default:
        {
          break;
        }
      }
      {
        i = (i + 1);
        if ((i >= 4)) { break; }
      }
      continue;
    }
  }
}
