struct main_inputs {
  uint3 global_id : SV_DispatchThreadID;
};


ByteAddressBuffer firstMatrix : register(t0);
ByteAddressBuffer secondMatrix : register(t1);
RWByteAddressBuffer resultMatrix : register(u2);
cbuffer cbuffer_uniforms : register(b3) {
  uint4 uniforms[2];
};
void main_inner(uint3 global_id) {
  uint2 resultCell = uint2(global_id.y, global_id.x);
  uint dimInner = uniforms[0u].y;
  uint dimOutter = uniforms[1u].y;
  uint result = 0u;
  {
    uint i = 0u;
    while(true) {
      if ((i < dimInner)) {
      } else {
        break;
      }
      uint a = (i + (resultCell.x * dimInner));
      uint b = (resultCell.y + (i * dimOutter));
      result = (result + (firstMatrix.Load((0u + (a * 4u))) * secondMatrix.Load((0u + (b * 4u)))));
      {
        i = (i + 1u);
      }
      continue;
    }
  }
  uint index = (resultCell.y + (resultCell.x * dimOutter));
  resultMatrix.Store((0u + (index * 4u)), result);
}

[numthreads(2, 2, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.global_id);
}

