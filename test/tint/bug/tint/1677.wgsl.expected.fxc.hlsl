struct main_inputs {
  uint3 id : SV_DispatchThreadID;
};


ByteAddressBuffer input : register(t0);
void main_inner(uint3 id) {
  uint3 v = asuint(asint(input.Load3(0u)));
  int3 pos = asint((v - asuint(int3((int(0)).xxx))));
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.id);
}

