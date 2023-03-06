#version 310 es

int tint_workgroupUniformLoad(inout int p) {
  barrier();
  int result = p;
  barrier();
  return result;
}

shared int arg_0;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  int inner;
} prevent_dce;

void workgroupUniformLoad_9d33de() {
  int res = tint_workgroupUniformLoad(arg_0);
  prevent_dce.inner = res;
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
