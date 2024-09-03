#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int j = 0;
  {
    int i = 0;
    while(true) {
      if ((i < 2)) {
      } else {
        break;
      }
      switch(i) {
        case 0:
        {
          switch(j) {
            case 0:
            {
              {
                i = (i + 2);
              }
              continue;
            }
            default:
            {
              break;
            }
          }
          break;
        }
        default:
        {
          break;
        }
      }
      {
        i = (i + 2);
      }
      continue;
    }
  }
}
