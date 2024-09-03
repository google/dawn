#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  {
    int i = 0;
    while(true) {
      if ((i < 2)) {
      } else {
        break;
      }
      {
        int j = 0;
        while(true) {
          if ((j < 2)) {
          } else {
            break;
          }
          switch(i) {
            case 0:
            {
              {
                j = (j + 2);
              }
              continue;
            }
            default:
            {
              break;
            }
          }
          {
            j = (j + 2);
          }
          continue;
        }
      }
      {
        i = (i + 2);
      }
      continue;
    }
  }
}
