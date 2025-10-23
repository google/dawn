#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int a_1 = 0;
  switch(a_1) {
    case 0:
    {
      break;
    }
    case 1:
    {
      return;
    }
    default:
    {
      uint v = uint(a_1);
      a_1 = int((v + uint(2)));
      break;
    }
  }
}
