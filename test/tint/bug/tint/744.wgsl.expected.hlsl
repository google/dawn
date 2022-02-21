ByteAddressBuffer firstMatrix : register(t0, space0);
ByteAddressBuffer secondMatrix : register(t1, space0);
RWByteAddressBuffer resultMatrix : register(u2, space0);
cbuffer cbuffer_uniforms : register(b3, space0) {
  uint4 uniforms[2];
};

struct tint_symbol_1 {
  uint3 global_id : SV_DispatchThreadID;
};

void main_inner(uint3 global_id) {
  const uint2 resultCell = uint2(global_id.y, global_id.x);
  const uint dimInner = uniforms[0].y;
  const uint dimOutter = uniforms[1].y;
  uint result = 0u;
  {
    [loop] for(uint i = 0u; (i < dimInner); i = (i + 1u)) {
      const uint a = (i + (resultCell.x * dimInner));
      const uint b = (resultCell.y + (i * dimOutter));
      result = (result + (firstMatrix.Load((4u * a)) * secondMatrix.Load((4u * b))));
    }
  }
  const uint index = (resultCell.y + (resultCell.x * dimOutter));
  resultMatrix.Store((4u * index), asuint(result));
}

[numthreads(2, 2, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.global_id);
  return;
}
