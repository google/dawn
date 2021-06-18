ByteAddressBuffer firstMatrix : register(t0, space0);
ByteAddressBuffer secondMatrix : register(t1, space0);
RWByteAddressBuffer resultMatrix : register(u2, space0);
cbuffer cbuffer_uniforms : register(b3, space0) {
  uint4 uniforms[2];
};

struct tint_symbol_1 {
  uint3 global_id : SV_DispatchThreadID;
};

[numthreads(2, 2, 1)]
void main(tint_symbol_1 tint_symbol) {
  const uint3 global_id = tint_symbol.global_id;
  const uint2 resultCell = uint2(global_id.y, global_id.x);
  const int scalar_offset = (4u) / 4;
  const uint dimInner = uniforms[scalar_offset / 4][scalar_offset % 4];
  const int scalar_offset_1 = (20u) / 4;
  const uint dimOutter = uniforms[scalar_offset_1 / 4][scalar_offset_1 % 4];
  uint result = 0u;
  {
    uint i = 0u;
    while (true) {
      if (!((i < dimInner))) {
        break;
      }
      const uint a = (i + (resultCell.x * dimInner));
      const uint b = (resultCell.y + (i * dimOutter));
      result = (result + (firstMatrix.Load((4u * a)) * secondMatrix.Load((4u * b))));
      {
        i = (i + 1u);
      }
    }
  }
  const uint index = (resultCell.y + (resultCell.x * dimOutter));
  resultMatrix.Store((4u * index), asuint(result));
  return;
}
