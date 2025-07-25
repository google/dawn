struct VSOutputs {
  int result;
  float4 position;
};

struct VSInput {
  float4 val;
};

struct vsMain_outputs {
  nointerpolation int VSOutputs_result : TEXCOORD0;
  float4 VSOutputs_position : SV_Position;
};

struct vsMain_inputs {
  float4 VSInput_val : TEXCOORD0;
};


VSOutputs vsMain_inner(VSInput vtxIn) {
  VSOutputs v = {int(1), vtxIn.val};
  return v;
}

vsMain_outputs vsMain(vsMain_inputs inputs) {
  VSInput v_1 = {inputs.VSInput_val};
  VSOutputs v_2 = vsMain_inner(v_1);
  vsMain_outputs v_3 = {v_2.result, v_2.position};
  return v_3;
}

