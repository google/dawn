
cbuffer cbuffer_U : register(b0) {
  uint4 U[1];
};
void f() {
  uint3 v = U[0u].xyz;
  uint x = U[0u].x;
  uint y = U[0u].y;
  uint z = U[0u].z;
  uint2 xx = U[0u].xyz.xx;
  uint2 xy = U[0u].xyz.xy;
  uint2 xz = U[0u].xyz.xz;
  uint2 yx = U[0u].xyz.yx;
  uint2 yy = U[0u].xyz.yy;
  uint2 yz = U[0u].xyz.yz;
  uint2 zx = U[0u].xyz.zx;
  uint2 zy = U[0u].xyz.zy;
  uint2 zz = U[0u].xyz.zz;
  uint3 xxx = U[0u].xyz.xxx;
  uint3 xxy = U[0u].xyz.xxy;
  uint3 xxz = U[0u].xyz.xxz;
  uint3 xyx = U[0u].xyz.xyx;
  uint3 xyy = U[0u].xyz.xyy;
  uint3 xyz = U[0u].xyz.xyz;
  uint3 xzx = U[0u].xyz.xzx;
  uint3 xzy = U[0u].xyz.xzy;
  uint3 xzz = U[0u].xyz.xzz;
  uint3 yxx = U[0u].xyz.yxx;
  uint3 yxy = U[0u].xyz.yxy;
  uint3 yxz = U[0u].xyz.yxz;
  uint3 yyx = U[0u].xyz.yyx;
  uint3 yyy = U[0u].xyz.yyy;
  uint3 yyz = U[0u].xyz.yyz;
  uint3 yzx = U[0u].xyz.yzx;
  uint3 yzy = U[0u].xyz.yzy;
  uint3 yzz = U[0u].xyz.yzz;
  uint3 zxx = U[0u].xyz.zxx;
  uint3 zxy = U[0u].xyz.zxy;
  uint3 zxz = U[0u].xyz.zxz;
  uint3 zyx = U[0u].xyz.zyx;
  uint3 zyy = U[0u].xyz.zyy;
  uint3 zyz = U[0u].xyz.zyz;
  uint3 zzx = U[0u].xyz.zzx;
  uint3 zzy = U[0u].xyz.zzy;
  uint3 zzz = U[0u].xyz.zzz;
  uint4 xxxx = U[0u].xyz.xxxx;
  uint4 xxxy = U[0u].xyz.xxxy;
  uint4 xxxz = U[0u].xyz.xxxz;
  uint4 xxyx = U[0u].xyz.xxyx;
  uint4 xxyy = U[0u].xyz.xxyy;
  uint4 xxyz = U[0u].xyz.xxyz;
  uint4 xxzx = U[0u].xyz.xxzx;
  uint4 xxzy = U[0u].xyz.xxzy;
  uint4 xxzz = U[0u].xyz.xxzz;
  uint4 xyxx = U[0u].xyz.xyxx;
  uint4 xyxy = U[0u].xyz.xyxy;
  uint4 xyxz = U[0u].xyz.xyxz;
  uint4 xyyx = U[0u].xyz.xyyx;
  uint4 xyyy = U[0u].xyz.xyyy;
  uint4 xyyz = U[0u].xyz.xyyz;
  uint4 xyzx = U[0u].xyz.xyzx;
  uint4 xyzy = U[0u].xyz.xyzy;
  uint4 xyzz = U[0u].xyz.xyzz;
  uint4 xzxx = U[0u].xyz.xzxx;
  uint4 xzxy = U[0u].xyz.xzxy;
  uint4 xzxz = U[0u].xyz.xzxz;
  uint4 xzyx = U[0u].xyz.xzyx;
  uint4 xzyy = U[0u].xyz.xzyy;
  uint4 xzyz = U[0u].xyz.xzyz;
  uint4 xzzx = U[0u].xyz.xzzx;
  uint4 xzzy = U[0u].xyz.xzzy;
  uint4 xzzz = U[0u].xyz.xzzz;
  uint4 yxxx = U[0u].xyz.yxxx;
  uint4 yxxy = U[0u].xyz.yxxy;
  uint4 yxxz = U[0u].xyz.yxxz;
  uint4 yxyx = U[0u].xyz.yxyx;
  uint4 yxyy = U[0u].xyz.yxyy;
  uint4 yxyz = U[0u].xyz.yxyz;
  uint4 yxzx = U[0u].xyz.yxzx;
  uint4 yxzy = U[0u].xyz.yxzy;
  uint4 yxzz = U[0u].xyz.yxzz;
  uint4 yyxx = U[0u].xyz.yyxx;
  uint4 yyxy = U[0u].xyz.yyxy;
  uint4 yyxz = U[0u].xyz.yyxz;
  uint4 yyyx = U[0u].xyz.yyyx;
  uint4 yyyy = U[0u].xyz.yyyy;
  uint4 yyyz = U[0u].xyz.yyyz;
  uint4 yyzx = U[0u].xyz.yyzx;
  uint4 yyzy = U[0u].xyz.yyzy;
  uint4 yyzz = U[0u].xyz.yyzz;
  uint4 yzxx = U[0u].xyz.yzxx;
  uint4 yzxy = U[0u].xyz.yzxy;
  uint4 yzxz = U[0u].xyz.yzxz;
  uint4 yzyx = U[0u].xyz.yzyx;
  uint4 yzyy = U[0u].xyz.yzyy;
  uint4 yzyz = U[0u].xyz.yzyz;
  uint4 yzzx = U[0u].xyz.yzzx;
  uint4 yzzy = U[0u].xyz.yzzy;
  uint4 yzzz = U[0u].xyz.yzzz;
  uint4 zxxx = U[0u].xyz.zxxx;
  uint4 zxxy = U[0u].xyz.zxxy;
  uint4 zxxz = U[0u].xyz.zxxz;
  uint4 zxyx = U[0u].xyz.zxyx;
  uint4 zxyy = U[0u].xyz.zxyy;
  uint4 zxyz = U[0u].xyz.zxyz;
  uint4 zxzx = U[0u].xyz.zxzx;
  uint4 zxzy = U[0u].xyz.zxzy;
  uint4 zxzz = U[0u].xyz.zxzz;
  uint4 zyxx = U[0u].xyz.zyxx;
  uint4 zyxy = U[0u].xyz.zyxy;
  uint4 zyxz = U[0u].xyz.zyxz;
  uint4 zyyx = U[0u].xyz.zyyx;
  uint4 zyyy = U[0u].xyz.zyyy;
  uint4 zyyz = U[0u].xyz.zyyz;
  uint4 zyzx = U[0u].xyz.zyzx;
  uint4 zyzy = U[0u].xyz.zyzy;
  uint4 zyzz = U[0u].xyz.zyzz;
  uint4 zzxx = U[0u].xyz.zzxx;
  uint4 zzxy = U[0u].xyz.zzxy;
  uint4 zzxz = U[0u].xyz.zzxz;
  uint4 zzyx = U[0u].xyz.zzyx;
  uint4 zzyy = U[0u].xyz.zzyy;
  uint4 zzyz = U[0u].xyz.zzyz;
  uint4 zzzx = U[0u].xyz.zzzx;
  uint4 zzzy = U[0u].xyz.zzzy;
  uint4 zzzz = U[0u].xyz.zzzz;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

