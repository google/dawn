struct ComputeInputs {
  uint3 local_invocation_id;
  uint local_invocation_index;
  uint3 global_invocation_id;
  uint3 workgroup_id;
};
struct tint_symbol_1 {
  uint3 local_invocation_id : SV_GroupThreadID;
  uint local_invocation_index : SV_GroupIndex;
  uint3 global_invocation_id : SV_DispatchThreadID;
  uint3 workgroup_id : SV_GroupID;
};

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  const ComputeInputs inputs = {tint_symbol.local_invocation_id, tint_symbol.local_invocation_index, tint_symbol.global_invocation_id, tint_symbol.workgroup_id};
  const uint foo = (((inputs.local_invocation_id.x + inputs.local_invocation_index) + inputs.global_invocation_id.x) + inputs.workgroup_id.x);
  return;
}
