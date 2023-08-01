SKIP: FAILED

#version 310 es

layout(binding = 0, std430) buffer tint_symbol_block_ssbo {
  uint inner[];
} tint_symbol;

struct ComputeInputs {
  uint subgroup_invocation_id;
  uint subgroup_size;
};

void tint_symbol_1(ComputeInputs inputs) {
  tint_symbol.inner[inputs.subgroup_invocation_id] = inputs.subgroup_size;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  ComputeInputs tint_symbol_4 = ComputeInputs(tint_symbol_2, tint_symbol_3);
  tint_symbol_1(tint_symbol_4);
  return;
}
Error parsing GLSL shader:
ERROR: 0:18: 'tint_symbol_2' : undeclared identifier
ERROR: 0:18: '' : compilation terminated
ERROR: 2 compilation errors.  No code generated.



