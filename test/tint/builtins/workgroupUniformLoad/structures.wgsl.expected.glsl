#version 310 es


struct Inner {
  bool b;
  ivec4 v;
  mat3 m;
};

struct Outer {
  Inner a[4];
};

shared Outer v;
Outer foo() {
  barrier();
  Outer v_1 = v;
  barrier();
  return v_1;
}
void main_inner(uint tint_local_index) {
  {
    uint v_2 = 0u;
    v_2 = tint_local_index;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 4u)) {
        break;
      }
      v.a[v_3] = Inner(false, ivec4(0), mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)));
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
  barrier();
  foo();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
