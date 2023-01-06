#version 310 es

shared int arg_0;
int tint_workgroupUniformLoad_arg_0() {
  barrier();
  int result = arg_0;
  barrier();
  return result;
}

void workgroupUniformLoad_9d33de() {
  int res = tint_workgroupUniformLoad_arg_0();
}

void compute_main(uint local_invocation_index) {
  {
    arg_0 = 0;
  }
  barrier();
  workgroupUniformLoad_9d33de();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main(gl_LocalInvocationIndex);
  return;
}
