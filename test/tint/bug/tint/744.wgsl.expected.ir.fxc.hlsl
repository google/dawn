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
  uint2 resultCell = uint2(global_id[1u], global_id[0u]);
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
      uint a = (i + (resultCell[0u] * dimInner));
      uint b = (resultCell[1u] + (i * dimOutter));
      uint v = result;
      uint v_1 = firstMatrix.Load((0u + (uint(a) * 4u)));
      result = (v + (v_1 * secondMatrix.Load((0u + (uint(b) * 4u)))));
      {
        i = (i + 1u);
      }
      continue;
    }
  }
  uint index = (resultCell[1u] + (resultCell[0u] * dimOutter));
  uint v_2 = (uint(index) * 4u);
  resultMatrix.Store((0u + v_2), result);
}

[numthreads(2, 2, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.global_id);
}

