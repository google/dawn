#version 310 es


struct Buf {
  uvec4 res;
};

layout(binding = 0, std430)
buffer buf_block_1_ssbo {
  Buf inner;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec4 v = uvec4(1u, 2u, 3u, 4u);
  uvec4 v_2 = v;
  v = uvec4(v_2.x, uvec2(5u, 6u).y, uvec2(5u, 6u).x, v_2.w);
  uvec4 v_3 = v;
  v = uvec4(v_3.x, uvec2(99u, 100u).y, uvec2(99u, 100u).x, v_3.w);
  v.z = 200u;
  uvec3 v_4 = (v.xyz * uvec3(100u));
  v = uvec4(v_4.x, v_4.y, v_4.z, v.w);
  v_1.inner.res = uvec4(uvec4(0u).x, uvec4(0u).y, uvec4(0u).z, uvec4(0u).w);
  uvec4 v_5 = v_1.inner.res;
  v_1.inner.res = uvec4(v_5.x, uvec2(1u, 2u).y, uvec2(1u, 2u).x, v_5.w);
  uvec4 v_6 = v_1.inner.res;
  v_1.inner.res = uvec4(v_6.x, uvec2(3u, 4u).y, uvec2(3u, 4u).x, v_6.w);
  v_1.inner.res.z = 5u;
  uvec3 v_7 = (v_1.inner.res.xyz + uvec3(10u));
  v_1.inner.res = uvec4(v_7.x, v_7.y, v_7.z, v_1.inner.res.w);
}
