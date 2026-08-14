
RWByteAddressBuffer buf : register(u0);
[numthreads(1, 1, 1)]
void main() {
  uint idx = 0u;
  uint4 v = uint4(1u, 2u, 3u, 4u);
  uint4 v_1 = v;
  v = uint4(v_1.x, uint2(5u, 6u).y, uint2(5u, 6u).x, v_1.w);
  uint4 v_2 = v;
  v = uint4(v_2.x, uint2(99u, 100u).y, uint2(99u, 100u).x, v_2.w);
  v.z = 200u;
  uint3 v_3 = (v.xyz * (100u).xxx);
  v = uint4(v_3.x, v_3.y, v_3.z, v.w);
  v.y = 300u;
  uint v_4[3] = {2u, 1u, 0u};
  v[min(v_4[min(idx, 2u)], 3u)] = 400u;
  v.x = (v.x + 500u);
  buf.Load4(0u);
  buf.Store4(0u, uint4((0u).xxxx.x, (0u).xxxx.y, (0u).xxxx.z, (0u).xxxx.w));
  uint4 v_5 = buf.Load4(0u);
  buf.Store4(0u, uint4(v_5.x, uint2(1u, 2u).y, uint2(1u, 2u).x, v_5.w));
  uint4 v_6 = buf.Load4(0u);
  buf.Store4(0u, uint4(v_6.x, uint2(3u, 4u).y, uint2(3u, 4u).x, v_6.w));
  buf.Store(8u, 5u);
  uint3 v_7 = (buf.Load4(0u).xyz + (10u).xxx);
  buf.Store4(0u, uint4(v_7.x, v_7.y, v_7.z, buf.Load4(0u).w));
  buf.Store(4u, 5u);
  buf.Store((0u + (min(v_4[min(idx, 2u)], 3u) * 4u)), 6u);
  buf.Store(0u, (buf.Load(0u) + 7u));
}

