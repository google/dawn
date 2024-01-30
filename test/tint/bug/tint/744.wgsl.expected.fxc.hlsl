ByteAddressBuffer firstMatrix : register(t0);
ByteAddressBuffer secondMatrix : register(t1);
RWByteAddressBuffer resultMatrix : register(u2);
cbuffer cbuffer_uniforms : register(b3) {
  uint4 uniforms[2];
};

struct tint_symbol_1 {
  uint3 global_id : SV_DispatchThreadID;
};

void main_inner(uint3 global_id) {
  uint2 resultCell = uint2(global_id.y, global_id.x);
  uint dimInner = uniforms[0].y;
  uint dimOutter = uniforms[1].y;
  uint result = 0u;
  {
    for(uint i = 0u; (i < dimInner); i = (i + 1u)) {
      uint a = (i + (resultCell.x * dimInner));
      uint b = (resultCell.y + (i * dimOutter));
      result = (result + (firstMatrix.Load((4u * a)) * secondMatrix.Load((4u * b))));
    }
  }
  uint index = (resultCell.y + (resultCell.x * dimOutter));
  resultMatrix.Store((4u * index), asuint(result));
}

[numthreads(2, 2, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.global_id);
  return;
}
