#version 310 es

float tint_workgroupUniformLoad(inout float p) {
  barrier();
  float result = p;
  barrier();
  return result;
}

shared float arg_0;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void workgroupUniformLoad_7a857c() {
  float res = tint_workgroupUniformLoad(arg_0);
  prevent_dce.inner = res;
}

void compute_main(uint local_invocation_index) {
  {
    arg_0 = 0.0f;
  }
  barrier();
  workgroupUniformLoad_7a857c();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main(gl_LocalInvocationIndex);
  return;
}
