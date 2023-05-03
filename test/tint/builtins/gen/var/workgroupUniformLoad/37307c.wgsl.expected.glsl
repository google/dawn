#version 310 es

shared uint arg_0;
uint tint_workgroupUniformLoad_arg_0() {
  barrier();
  uint result = arg_0;
  barrier();
  return result;
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

void workgroupUniformLoad_37307c() {
  uint res = tint_workgroupUniformLoad_arg_0();
  prevent_dce.inner = res;
}

void compute_main(uint local_invocation_index) {
  {
    arg_0 = 0u;
  }
  barrier();
  workgroupUniformLoad_37307c();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main(gl_LocalInvocationIndex);
  return;
}
