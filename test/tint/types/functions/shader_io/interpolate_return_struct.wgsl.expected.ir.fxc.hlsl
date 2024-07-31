struct Out {
  float4 pos;
  float none;
  float flat;
  float perspective_center;
  float perspective_centroid;
  float perspective_sample;
  float linear_center;
  float linear_centroid;
  float linear_sample;
};

struct main_outputs {
  float Out_none : TEXCOORD0;
  nointerpolation float Out_flat : TEXCOORD1;
  linear float Out_perspective_center : TEXCOORD2;
  linear centroid float Out_perspective_centroid : TEXCOORD3;
  linear sample float Out_perspective_sample : TEXCOORD4;
  noperspective float Out_linear_center : TEXCOORD5;
  noperspective centroid float Out_linear_centroid : TEXCOORD6;
  noperspective sample float Out_linear_sample : TEXCOORD7;
  float4 Out_pos : SV_Position;
};


Out main_inner() {
  Out v = (Out)0;
  return v;
}

main_outputs main() {
  Out v_1 = main_inner();
  Out v_2 = v_1;
  Out v_3 = v_1;
  Out v_4 = v_1;
  Out v_5 = v_1;
  Out v_6 = v_1;
  Out v_7 = v_1;
  Out v_8 = v_1;
  Out v_9 = v_1;
  Out v_10 = v_1;
  main_outputs v_11 = {v_3.none, v_4.flat, v_5.perspective_center, v_6.perspective_centroid, v_7.perspective_sample, v_8.linear_center, v_9.linear_centroid, v_10.linear_sample, v_2.pos};
  return v_11;
}

