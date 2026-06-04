
RWByteAddressBuffer value : register(u0);
[numthreads(1, 1, 1)]
void main() {
  uint2 v = value.Load2(0u);
  uint2 v_1 = select(((v & (4294901760u).xx) == (0u).xx), (0u).xx, (16u).xx);
  uint2 v_2 = (v >> v_1);
  uint2 v_3 = select(((v_2 & (65280u).xx) == (0u).xx), (0u).xx, (8u).xx);
  uint2 v_4 = (v_2 >> v_3);
  uint2 v_5 = select(((v_4 & (240u).xx) == (0u).xx), (0u).xx, (4u).xx);
  uint2 v_6 = (v_4 >> v_5);
  uint2 v_7 = select(((v_6 & (12u).xx) == (0u).xx), (0u).xx, (2u).xx);
  uint2 v_8 = (v_6 >> v_7);
  uint2 v_9 = select((v_8 == (0u).xx), (4294967295u).xx, (v_1 | (v_3 | (v_5 | (v_7 | select(((v_8 & (2u).xx) == (0u).xx), (0u).xx, (1u).xx))))));
  uint2 v_10 = select(((v_9 & (4294901760u).xx) == (0u).xx), (0u).xx, (16u).xx);
  uint2 v_11 = (v_9 >> v_10);
  uint2 v_12 = select(((v_11 & (65280u).xx) == (0u).xx), (0u).xx, (8u).xx);
  uint2 v_13 = (v_11 >> v_12);
  uint2 v_14 = select(((v_13 & (240u).xx) == (0u).xx), (0u).xx, (4u).xx);
  uint2 v_15 = (v_13 >> v_14);
  uint2 v_16 = select(((v_15 & (12u).xx) == (0u).xx), (0u).xx, (2u).xx);
  uint2 v_17 = (v_15 >> v_16);
  uint2 v_18 = select((v_17 == (0u).xx), (4294967295u).xx, (v_10 | (v_12 | (v_14 | (v_16 | select(((v_17 & (2u).xx) == (0u).xx), (0u).xx, (1u).xx))))));
  uint2 v_19 = select(((v_18 & (4294901760u).xx) == (0u).xx), (0u).xx, (16u).xx);
  uint2 v_20 = (v_18 >> v_19);
  uint2 v_21 = select(((v_20 & (65280u).xx) == (0u).xx), (0u).xx, (8u).xx);
  uint2 v_22 = (v_20 >> v_21);
  uint2 v_23 = select(((v_22 & (240u).xx) == (0u).xx), (0u).xx, (4u).xx);
  uint2 v_24 = (v_22 >> v_23);
  uint2 v_25 = select(((v_24 & (12u).xx) == (0u).xx), (0u).xx, (2u).xx);
  uint2 v_26 = (v_24 >> v_25);
  uint2 v_27 = select((v_26 == (0u).xx), (4294967295u).xx, (v_19 | (v_21 | (v_23 | (v_25 | select(((v_26 & (2u).xx) == (0u).xx), (0u).xx, (1u).xx))))));
  uint2 v_28 = select(((v_27 & (4294901760u).xx) == (0u).xx), (0u).xx, (16u).xx);
  uint2 v_29 = (v_27 >> v_28);
  uint2 v_30 = select(((v_29 & (65280u).xx) == (0u).xx), (0u).xx, (8u).xx);
  uint2 v_31 = (v_29 >> v_30);
  uint2 v_32 = select(((v_31 & (240u).xx) == (0u).xx), (0u).xx, (4u).xx);
  uint2 v_33 = (v_31 >> v_32);
  uint2 v_34 = select(((v_33 & (12u).xx) == (0u).xx), (0u).xx, (2u).xx);
  uint2 v_35 = (v_33 >> v_34);
  value.Store2(0u, select((v_35 == (0u).xx), (4294967295u).xx, (v_28 | (v_30 | (v_32 | (v_34 | select(((v_35 & (2u).xx) == (0u).xx), (0u).xx, (1u).xx)))))));
}

