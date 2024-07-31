struct computeMain_inputs {
  uint3 global_id : SV_DispatchThreadID;
};


RWByteAddressBuffer drawOut : register(u5);
static uint cubeVerts = 0u;
void computeMain_inner(uint3 global_id) {
  uint v = cubeVerts;
  uint v_1 = 0u;
  drawOut.InterlockedAdd(uint(0u), v, v_1);
  uint firstVertex = v_1;
}

[numthreads(1, 1, 1)]
void computeMain(computeMain_inputs inputs) {
  computeMain_inner(inputs.global_id);
}

