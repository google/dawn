// flags:  --hlsl-shader-model 62
enable f16;
struct S {
    v: vec3<f16>,
};

@group(0) @binding(0) var<uniform> U : S;

fn f() {
    var    v = U.v;

    var    x = U.v.x;
    var    y = U.v.y;
    var    z = U.v.z;

    var   xx = U.v.xx;
    var   xy = U.v.xy;
    var   xz = U.v.xz;
    var   yx = U.v.yx;
    var   yy = U.v.yy;
    var   yz = U.v.yz;
    var   zx = U.v.zx;
    var   zy = U.v.zy;
    var   zz = U.v.zz;

    var  xxx = U.v.xxx;
    var  xxy = U.v.xxy;
    var  xxz = U.v.xxz;
    var  xyx = U.v.xyx;
    var  xyy = U.v.xyy;
    var  xyz = U.v.xyz;
    var  xzx = U.v.xzx;
    var  xzy = U.v.xzy;
    var  xzz = U.v.xzz;

    var  yxx = U.v.yxx;
    var  yxy = U.v.yxy;
    var  yxz = U.v.yxz;
    var  yyx = U.v.yyx;
    var  yyy = U.v.yyy;
    var  yyz = U.v.yyz;
    var  yzx = U.v.yzx;
    var  yzy = U.v.yzy;
    var  yzz = U.v.yzz;

    var  zxx = U.v.zxx;
    var  zxy = U.v.zxy;
    var  zxz = U.v.zxz;
    var  zyx = U.v.zyx;
    var  zyy = U.v.zyy;
    var  zyz = U.v.zyz;
    var  zzx = U.v.zzx;
    var  zzy = U.v.zzy;
    var  zzz = U.v.zzz;

    var xxxx = U.v.xxxx;
    var xxxy = U.v.xxxy;
    var xxxz = U.v.xxxz;
    var xxyx = U.v.xxyx;
    var xxyy = U.v.xxyy;
    var xxyz = U.v.xxyz;
    var xxzx = U.v.xxzx;
    var xxzy = U.v.xxzy;
    var xxzz = U.v.xxzz;

    var xyxx = U.v.xyxx;
    var xyxy = U.v.xyxy;
    var xyxz = U.v.xyxz;
    var xyyx = U.v.xyyx;
    var xyyy = U.v.xyyy;
    var xyyz = U.v.xyyz;
    var xyzx = U.v.xyzx;
    var xyzy = U.v.xyzy;
    var xyzz = U.v.xyzz;

    var xzxx = U.v.xzxx;
    var xzxy = U.v.xzxy;
    var xzxz = U.v.xzxz;
    var xzyx = U.v.xzyx;
    var xzyy = U.v.xzyy;
    var xzyz = U.v.xzyz;
    var xzzx = U.v.xzzx;
    var xzzy = U.v.xzzy;
    var xzzz = U.v.xzzz;

    var yxxx = U.v.yxxx;
    var yxxy = U.v.yxxy;
    var yxxz = U.v.yxxz;
    var yxyx = U.v.yxyx;
    var yxyy = U.v.yxyy;
    var yxyz = U.v.yxyz;
    var yxzx = U.v.yxzx;
    var yxzy = U.v.yxzy;
    var yxzz = U.v.yxzz;

    var yyxx = U.v.yyxx;
    var yyxy = U.v.yyxy;
    var yyxz = U.v.yyxz;
    var yyyx = U.v.yyyx;
    var yyyy = U.v.yyyy;
    var yyyz = U.v.yyyz;
    var yyzx = U.v.yyzx;
    var yyzy = U.v.yyzy;
    var yyzz = U.v.yyzz;

    var yzxx = U.v.yzxx;
    var yzxy = U.v.yzxy;
    var yzxz = U.v.yzxz;
    var yzyx = U.v.yzyx;
    var yzyy = U.v.yzyy;
    var yzyz = U.v.yzyz;
    var yzzx = U.v.yzzx;
    var yzzy = U.v.yzzy;
    var yzzz = U.v.yzzz;

    var zxxx = U.v.zxxx;
    var zxxy = U.v.zxxy;
    var zxxz = U.v.zxxz;
    var zxyx = U.v.zxyx;
    var zxyy = U.v.zxyy;
    var zxyz = U.v.zxyz;
    var zxzx = U.v.zxzx;
    var zxzy = U.v.zxzy;
    var zxzz = U.v.zxzz;

    var zyxx = U.v.zyxx;
    var zyxy = U.v.zyxy;
    var zyxz = U.v.zyxz;
    var zyyx = U.v.zyyx;
    var zyyy = U.v.zyyy;
    var zyyz = U.v.zyyz;
    var zyzx = U.v.zyzx;
    var zyzy = U.v.zyzy;
    var zyzz = U.v.zyzz;

    var zzxx = U.v.zzxx;
    var zzxy = U.v.zzxy;
    var zzxz = U.v.zzxz;
    var zzyx = U.v.zzyx;
    var zzyy = U.v.zzyy;
    var zzyz = U.v.zzyz;
    var zzzx = U.v.zzzx;
    var zzzy = U.v.zzzy;
    var zzzz = U.v.zzzz;
}
