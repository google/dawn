#version 310 es

layout(binding = 0, std430)
buffer value_block_1_ssbo {
  uvec2 inner;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec2 v_1 = v.inner;
  uvec2 v_2 = mix(uvec2(16u), uvec2(0u), equal((v_1 & uvec2(4294901760u)), uvec2(0u)));
  uvec2 v_3 = (v_1 >> v_2);
  uvec2 v_4 = mix(uvec2(8u), uvec2(0u), equal((v_3 & uvec2(65280u)), uvec2(0u)));
  uvec2 v_5 = (v_3 >> v_4);
  uvec2 v_6 = mix(uvec2(4u), uvec2(0u), equal((v_5 & uvec2(240u)), uvec2(0u)));
  uvec2 v_7 = (v_5 >> v_6);
  uvec2 v_8 = mix(uvec2(2u), uvec2(0u), equal((v_7 & uvec2(12u)), uvec2(0u)));
  uvec2 v_9 = (v_7 >> v_8);
  uvec2 v_10 = mix((v_2 | (v_4 | (v_6 | (v_8 | mix(uvec2(1u), uvec2(0u), equal((v_9 & uvec2(2u)), uvec2(0u))))))), uvec2(4294967295u), equal(v_9, uvec2(0u)));
  uvec2 v_11 = mix(uvec2(16u), uvec2(0u), equal((v_10 & uvec2(4294901760u)), uvec2(0u)));
  uvec2 v_12 = (v_10 >> v_11);
  uvec2 v_13 = mix(uvec2(8u), uvec2(0u), equal((v_12 & uvec2(65280u)), uvec2(0u)));
  uvec2 v_14 = (v_12 >> v_13);
  uvec2 v_15 = mix(uvec2(4u), uvec2(0u), equal((v_14 & uvec2(240u)), uvec2(0u)));
  uvec2 v_16 = (v_14 >> v_15);
  uvec2 v_17 = mix(uvec2(2u), uvec2(0u), equal((v_16 & uvec2(12u)), uvec2(0u)));
  uvec2 v_18 = (v_16 >> v_17);
  uvec2 v_19 = mix((v_11 | (v_13 | (v_15 | (v_17 | mix(uvec2(1u), uvec2(0u), equal((v_18 & uvec2(2u)), uvec2(0u))))))), uvec2(4294967295u), equal(v_18, uvec2(0u)));
  uvec2 v_20 = mix(uvec2(16u), uvec2(0u), equal((v_19 & uvec2(4294901760u)), uvec2(0u)));
  uvec2 v_21 = (v_19 >> v_20);
  uvec2 v_22 = mix(uvec2(8u), uvec2(0u), equal((v_21 & uvec2(65280u)), uvec2(0u)));
  uvec2 v_23 = (v_21 >> v_22);
  uvec2 v_24 = mix(uvec2(4u), uvec2(0u), equal((v_23 & uvec2(240u)), uvec2(0u)));
  uvec2 v_25 = (v_23 >> v_24);
  uvec2 v_26 = mix(uvec2(2u), uvec2(0u), equal((v_25 & uvec2(12u)), uvec2(0u)));
  uvec2 v_27 = (v_25 >> v_26);
  uvec2 v_28 = mix((v_20 | (v_22 | (v_24 | (v_26 | mix(uvec2(1u), uvec2(0u), equal((v_27 & uvec2(2u)), uvec2(0u))))))), uvec2(4294967295u), equal(v_27, uvec2(0u)));
  uvec2 v_29 = mix(uvec2(16u), uvec2(0u), equal((v_28 & uvec2(4294901760u)), uvec2(0u)));
  uvec2 v_30 = (v_28 >> v_29);
  uvec2 v_31 = mix(uvec2(8u), uvec2(0u), equal((v_30 & uvec2(65280u)), uvec2(0u)));
  uvec2 v_32 = (v_30 >> v_31);
  uvec2 v_33 = mix(uvec2(4u), uvec2(0u), equal((v_32 & uvec2(240u)), uvec2(0u)));
  uvec2 v_34 = (v_32 >> v_33);
  uvec2 v_35 = mix(uvec2(2u), uvec2(0u), equal((v_34 & uvec2(12u)), uvec2(0u)));
  uvec2 v_36 = (v_34 >> v_35);
  v.inner = mix((v_29 | (v_31 | (v_33 | (v_35 | mix(uvec2(1u), uvec2(0u), equal((v_36 & uvec2(2u)), uvec2(0u))))))), uvec2(4294967295u), equal(v_36, uvec2(0u)));
}
