struct Mat4x4_ {
    mx: vec4<f32>;
    my: vec4<f32>;
    mz: vec4<f32>;
    mw: vec4<f32>;
};

struct Mat4x3_ {
    mx: vec4<f32>;
    my: vec4<f32>;
    mz: vec4<f32>;
};

struct Mat4x2_ {
    mx: vec4<f32>;
    my: vec4<f32>;
};

[[block]]
struct ub_SceneParams {
    u_Projection: Mat4x4_;
};

[[block]]
struct ub_MaterialParams {
    u_TexMtx: [[stride(32)]] array<Mat4x2_,1>;
    u_Misc0_: vec4<f32>;
};

[[block]]
struct ub_PacketParams {
    u_PosMtx: [[stride(48)]] array<Mat4x3_,32>;
};

struct VertexOutput {
    [[location(0)]] v_Color: vec4<f32>;
    [[location(1)]] v_TexCoord: vec2<f32>;
    [[builtin(position)]] member: vec4<f32>;
};

[[group(0), binding(0)]]
var<uniform> global: ub_SceneParams;
[[group(0), binding(1)]]
var<uniform> global1: ub_MaterialParams;
[[group(0), binding(2)]]
var<uniform> global2: ub_PacketParams;
var<private> a_Position1: vec3<f32>;
var<private> a_UV1: vec2<f32>;
var<private> a_Color1: vec4<f32>;
var<private> a_Normal1: vec3<f32>;
var<private> a_PosMtxIdx1: f32;
var<private> v_Color: vec4<f32>;
var<private> v_TexCoord: vec2<f32>;
var<private> gl_Position: vec4<f32>;

fn Mat4x3GetCol0_(m: Mat4x3_) -> vec3<f32> {
    var m1: Mat4x3_;

    m1 = m;
    let _e2: Mat4x3_ = m1;
    let _e5: Mat4x3_ = m1;
    let _e8: Mat4x3_ = m1;
    return vec3<f32>(_e2.mx.x, _e5.my.x, _e8.mz.x);
}

fn Mat4x3GetCol1_(m2: Mat4x3_) -> vec3<f32> {
    var m3: Mat4x3_;

    m3 = m2;
    let _e2: Mat4x3_ = m3;
    let _e5: Mat4x3_ = m3;
    let _e8: Mat4x3_ = m3;
    return vec3<f32>(_e2.mx.y, _e5.my.y, _e8.mz.y);
}

fn Mat4x3GetCol2_(m4: Mat4x3_) -> vec3<f32> {
    var m5: Mat4x3_;

    m5 = m4;
    let _e2: Mat4x3_ = m5;
    let _e5: Mat4x3_ = m5;
    let _e8: Mat4x3_ = m5;
    return vec3<f32>(_e2.mx.z, _e5.my.z, _e8.mz.z);
}

fn Mat4x3GetCol3_(m6: Mat4x3_) -> vec3<f32> {
    var m7: Mat4x3_;

    m7 = m6;
    let _e2: Mat4x3_ = m7;
    let _e5: Mat4x3_ = m7;
    let _e8: Mat4x3_ = m7;
    return vec3<f32>(_e2.mx.w, _e5.my.w, _e8.mz.w);
}

fn Mul(m8: Mat4x4_, v: vec4<f32>) -> vec4<f32> {
    var m9: Mat4x4_;
    var v1: vec4<f32>;

    m9 = m8;
    v1 = v;
    let _e4: Mat4x4_ = m9;
    let _e6: vec4<f32> = v1;
    let _e8: Mat4x4_ = m9;
    let _e10: vec4<f32> = v1;
    let _e12: Mat4x4_ = m9;
    let _e14: vec4<f32> = v1;
    let _e16: Mat4x4_ = m9;
    let _e18: vec4<f32> = v1;
    return vec4<f32>(dot(_e4.mx, _e6), dot(_e8.my, _e10), dot(_e12.mz, _e14), dot(_e16.mw, _e18));
}

fn Mul1(m10: Mat4x3_, v2: vec4<f32>) -> vec3<f32> {
    var m11: Mat4x3_;
    var v3: vec4<f32>;

    m11 = m10;
    v3 = v2;
    let _e4: Mat4x3_ = m11;
    let _e6: vec4<f32> = v3;
    let _e8: Mat4x3_ = m11;
    let _e10: vec4<f32> = v3;
    let _e12: Mat4x3_ = m11;
    let _e14: vec4<f32> = v3;
    return vec3<f32>(dot(_e4.mx, _e6), dot(_e8.my, _e10), dot(_e12.mz, _e14));
}

fn Mul2(m12: Mat4x2_, v4: vec4<f32>) -> vec2<f32> {
    var m13: Mat4x2_;
    var v5: vec4<f32>;

    m13 = m12;
    v5 = v4;
    let _e4: Mat4x2_ = m13;
    let _e6: vec4<f32> = v5;
    let _e8: Mat4x2_ = m13;
    let _e10: vec4<f32> = v5;
    return vec2<f32>(dot(_e4.mx, _e6), dot(_e8.my, _e10));
}

fn Mul3(v6: vec3<f32>, m14: Mat4x3_) -> vec4<f32> {
    var v7: vec3<f32>;
    var m15: Mat4x3_;

    v7 = v6;
    m15 = m14;
    let _e5: Mat4x3_ = m15;
    let _e6: vec3<f32> = Mat4x3GetCol0_(_e5);
    let _e7: vec3<f32> = v7;
    let _e10: Mat4x3_ = m15;
    let _e11: vec3<f32> = Mat4x3GetCol1_(_e10);
    let _e12: vec3<f32> = v7;
    let _e15: Mat4x3_ = m15;
    let _e16: vec3<f32> = Mat4x3GetCol2_(_e15);
    let _e17: vec3<f32> = v7;
    let _e20: Mat4x3_ = m15;
    let _e21: vec3<f32> = Mat4x3GetCol3_(_e20);
    let _e22: vec3<f32> = v7;
    return vec4<f32>(dot(_e6, _e7), dot(_e11, _e12), dot(_e16, _e17), dot(_e21, _e22));
}

fn _Mat4x4_(n: f32) -> Mat4x4_ {
    var n1: f32;
    var o: Mat4x4_;

    n1 = n;
    let _e4: f32 = n1;
    o.mx = vec4<f32>(_e4, 0.0, 0.0, 0.0);
    let _e11: f32 = n1;
    o.my = vec4<f32>(0.0, _e11, 0.0, 0.0);
    let _e18: f32 = n1;
    o.mz = vec4<f32>(0.0, 0.0, _e18, 0.0);
    let _e25: f32 = n1;
    o.mw = vec4<f32>(0.0, 0.0, 0.0, _e25);
    let _e27: Mat4x4_ = o;
    return _e27;
}

fn _Mat4x4_1(m16: Mat4x3_) -> Mat4x4_ {
    var m17: Mat4x3_;
    var o1: Mat4x4_;

    m17 = m16;
    let _e4: Mat4x4_ = _Mat4x4_(1.0);
    o1 = _e4;
    let _e7: Mat4x3_ = m17;
    o1.mx = _e7.mx;
    let _e10: Mat4x3_ = m17;
    o1.my = _e10.my;
    let _e13: Mat4x3_ = m17;
    o1.mz = _e13.mz;
    let _e15: Mat4x4_ = o1;
    return _e15;
}

fn _Mat4x4_2(m18: Mat4x2_) -> Mat4x4_ {
    var m19: Mat4x2_;
    var o2: Mat4x4_;

    m19 = m18;
    let _e4: Mat4x4_ = _Mat4x4_(1.0);
    o2 = _e4;
    let _e7: Mat4x2_ = m19;
    o2.mx = _e7.mx;
    let _e10: Mat4x2_ = m19;
    o2.my = _e10.my;
    let _e12: Mat4x4_ = o2;
    return _e12;
}

fn _Mat4x3_(n2: f32) -> Mat4x3_ {
    var n3: f32;
    var o3: Mat4x3_;

    n3 = n2;
    let _e4: f32 = n3;
    o3.mx = vec4<f32>(_e4, 0.0, 0.0, 0.0);
    let _e11: f32 = n3;
    o3.my = vec4<f32>(0.0, _e11, 0.0, 0.0);
    let _e18: f32 = n3;
    o3.mz = vec4<f32>(0.0, 0.0, _e18, 0.0);
    let _e21: Mat4x3_ = o3;
    return _e21;
}

fn _Mat4x3_1(m20: Mat4x4_) -> Mat4x3_ {
    var m21: Mat4x4_;
    var o4: Mat4x3_;

    m21 = m20;
    let _e4: Mat4x4_ = m21;
    o4.mx = _e4.mx;
    let _e7: Mat4x4_ = m21;
    o4.my = _e7.my;
    let _e10: Mat4x4_ = m21;
    o4.mz = _e10.mz;
    let _e12: Mat4x3_ = o4;
    return _e12;
}

fn main1() {
    var t_PosMtx: Mat4x3_;
    var t_TexSpaceCoord: vec2<f32>;

    let _e15: f32 = a_PosMtxIdx1;
    let _e18: Mat4x3_ = global2.u_PosMtx[i32(_e15)];
    t_PosMtx = _e18;
    let _e23: Mat4x3_ = t_PosMtx;
    let _e24: Mat4x4_ = _Mat4x4_1(_e23);
    let _e25: vec3<f32> = a_Position1;
    let _e29: Mat4x3_ = t_PosMtx;
    let _e30: Mat4x4_ = _Mat4x4_1(_e29);
    let _e31: vec3<f32> = a_Position1;
    let _e34: vec4<f32> = Mul(_e30, vec4<f32>(_e31, 1.0));
    let _e35: Mat4x4_ = global.u_Projection;
    let _e37: Mat4x3_ = t_PosMtx;
    let _e38: Mat4x4_ = _Mat4x4_1(_e37);
    let _e39: vec3<f32> = a_Position1;
    let _e43: Mat4x3_ = t_PosMtx;
    let _e44: Mat4x4_ = _Mat4x4_1(_e43);
    let _e45: vec3<f32> = a_Position1;
    let _e48: vec4<f32> = Mul(_e44, vec4<f32>(_e45, 1.0));
    let _e49: vec4<f32> = Mul(_e35, _e48);
    gl_Position = _e49;
    let _e50: vec4<f32> = a_Color1;
    v_Color = _e50;
    let _e52: vec4<f32> = global1.u_Misc0_;
    if ((_e52.x == 2.0)) {
        {
            let _e59: vec3<f32> = a_Normal1;
            let _e64: Mat4x2_ = global1.u_TexMtx[0];
            let _e65: vec3<f32> = a_Normal1;
            let _e68: vec2<f32> = Mul2(_e64, vec4<f32>(_e65, 1.0));
            v_TexCoord = _e68.xy;
            return;
        }
    } else {
        {
            let _e73: vec2<f32> = a_UV1;
            let _e79: Mat4x2_ = global1.u_TexMtx[0];
            let _e80: vec2<f32> = a_UV1;
            let _e84: vec2<f32> = Mul2(_e79, vec4<f32>(_e80, 1.0, 1.0));
            v_TexCoord = _e84.xy;
            return;
        }
    }
}

[[stage(vertex)]]
fn main([[location(0)]] a_Position: vec3<f32>, [[location(1)]] a_UV: vec2<f32>, [[location(2)]] a_Color: vec4<f32>, [[location(3)]] a_Normal: vec3<f32>, [[location(4)]] a_PosMtxIdx: f32) -> VertexOutput {
    a_Position1 = a_Position;
    a_UV1 = a_UV;
    a_Color1 = a_Color;
    a_Normal1 = a_Normal;
    a_PosMtxIdx1 = a_PosMtxIdx;
    main1();
    let _e11: vec4<f32> = v_Color;
    let _e13: vec2<f32> = v_TexCoord;
    let _e15: vec4<f32> = gl_Position;
    return VertexOutput(_e11, _e13, _e15);
}