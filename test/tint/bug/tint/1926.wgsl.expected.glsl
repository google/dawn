#version 310 es

shared uint sh_atomic_failed;
uint tint_workgroupUniformLoad_sh_atomic_failed() {
  barrier();
  uint result = sh_atomic_failed;
  barrier();
  return result;
}

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    sh_atomic_failed = 0u;
  }
  barrier();
}

layout(binding = 4, std430) buffer tint_symbol_block_ssbo {
  uint inner;
} tint_symbol;

void tint_symbol_1(uvec3 global_id, uvec3 local_id, uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  uint failed = tint_workgroupUniformLoad_sh_atomic_failed();
  if ((local_id.x == 0u)) {
    tint_symbol.inner = failed;
  }
}

layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_1(gl_GlobalInvocationID, gl_LocalInvocationID, gl_LocalInvocationIndex);
  return;
}
