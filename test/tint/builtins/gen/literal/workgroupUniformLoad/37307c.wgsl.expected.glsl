#version 310 es

uint tint_workgroupUniformLoad(inout uint p) {
  barrier();
  uint result = p;
  barrier();
  return result;
}

shared uint arg_0;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

void workgroupUniformLoad_37307c() {
  uint res = tint_workgroupUniformLoad(arg_0);
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
