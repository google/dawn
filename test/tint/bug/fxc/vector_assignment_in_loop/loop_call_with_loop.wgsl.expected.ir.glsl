#version 310 es

vec2 v2f = vec2(0.0f);
ivec3 v3i = ivec3(0);
uvec4 v4u = uvec4(0u);
bvec2 v2b = bvec2(false);
void foo() {
  {
    int i = 0;
    while(true) {
      if ((i < 2)) {
      } else {
        break;
      }
      v2f[i] = 1.0f;
      v3i[i] = 1;
      v4u[i] = 1u;
      v2b[i] = true;
      {
        i = (i + 1);
      }
      continue;
    }
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  {
    int i = 0;
    while(true) {
      if ((i < 2)) {
      } else {
        break;
      }
      foo();
      {
        i = (i + 1);
      }
      continue;
    }
  }
}
