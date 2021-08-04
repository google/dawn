static uint3 gl_WorkGroupID = uint3(0u, 0u, 0u);
ByteAddressBuffer x_13 : register(t2, space0);
RWByteAddressBuffer x_15 : register(u3, space0);
ByteAddressBuffer x_17 : register(t1, space0);
ByteAddressBuffer x_19 : register(t0, space0);

void main_1() {
  uint base_index_in = 0u;
  uint base_index_out = 0u;
  int index_in0 = 0;
  int index_in1 = 0;
  int index_out0 = 0;
  int index_out1 = 0;
  int condition_index = 0;
  int i = 0;
  int temp0 = 0;
  int temp1 = 0;
  const uint x_58 = gl_WorkGroupID.x;
  base_index_in = (128u * x_58);
  const uint x_61 = gl_WorkGroupID.x;
  base_index_out = (256u * x_61);
  index_in0 = 127;
  index_in1 = 383;
  index_out0 = 255;
  index_out1 = 383;
  condition_index = 0;
  i = 0;
  {
    for(; (i < 256); i = (i + 1)) {
      const int x_72 = asint(x_13.Load((4u * uint(condition_index))));
      if ((x_72 == 0)) {
        const uint x_77 = base_index_out;
        const int x_78 = index_out0;
        const int x_86 = asint(x_17.Load((4u * (base_index_in + asuint(index_in0)))));
        x_15.Store((4u * (x_77 + asuint(x_78))), asuint(x_86));
        index_out0 = (index_out0 - 1);
        index_in1 = (index_in1 - 1);
      } else {
        const uint x_92 = base_index_out;
        const int x_93 = index_out1;
        const int x_101 = asint(x_19.Load((4u * (base_index_in + asuint(index_in1)))));
        x_15.Store((4u * (x_92 + asuint(x_93))), asuint(x_101));
        index_out1 = (index_out1 - 1);
        index_in1 = (index_in1 - 1);
      }
      const int x_110 = asint(x_13.Load((4u * uint((condition_index + 1)))));
      condition_index = (condition_index + x_110);
      temp0 = index_in0;
      index_in0 = index_in1;
      index_in1 = temp0;
      temp1 = index_out0;
      index_out0 = index_out1;
      index_out1 = temp1;
    }
  }
  return;
}

struct tint_symbol_1 {
  uint3 gl_WorkGroupID_param : SV_GroupID;
};

void main_inner(uint3 gl_WorkGroupID_param) {
  gl_WorkGroupID = gl_WorkGroupID_param;
  main_1();
}

[numthreads(4, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.gl_WorkGroupID_param);
  return;
}
