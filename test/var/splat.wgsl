var<private> g_v2 : vec2<f32> = vec2<f32>(1.0);
var<private> g_v3 : vec3<f32> = vec3<f32>(1.0);
var<private> g_v4 : vec4<f32> = vec4<f32>(1.0);

fn from_immediate_bool() {
    var v2 : vec2<bool> = vec2<bool>(true);
    var v3 : vec3<bool> = vec3<bool>(true);
    var v4 : vec4<bool> = vec4<bool>(true);
}

fn from_immediate_f32() {
    var v2 : vec2<f32> = vec2<f32>(1.0);
    var v3 : vec3<f32> = vec3<f32>(1.0);
    var v4 : vec4<f32> = vec4<f32>(1.0);
}

fn from_immediate_i32() {
    var v2 : vec2<i32> = vec2<i32>(1);
    var v3 : vec3<i32> = vec3<i32>(1);
    var v4 : vec4<i32> = vec4<i32>(1);
}

fn from_immediate_u32() {
    var v2 : vec2<u32> = vec2<u32>(1u);
    var v3 : vec3<u32> = vec3<u32>(1u);
    var v4 : vec4<u32> = vec4<u32>(1u);
}

fn from_expression_bool() {
    var v2 : vec2<bool> = vec2<bool>(true);
    var v3 : vec3<bool> = vec3<bool>(true);
    var v4 : vec4<bool> = vec4<bool>(true);
}

fn from_expression_f32() {
    var v2 : vec2<f32> = vec2<f32>(1.0 + 2.0);
    var v3 : vec3<f32> = vec3<f32>(1.0 + 2.0);
    var v4 : vec4<f32> = vec4<f32>(1.0 + 2.0);
}

fn from_expression_i32() {
    var v2 : vec2<i32> = vec2<i32>(1 + 2);
    var v3 : vec3<i32> = vec3<i32>(1 + 2);
    var v4 : vec4<i32> = vec4<i32>(1 + 2);
}

fn from_expression_u32() {
    var v2 : vec2<u32> = vec2<u32>(1u + 2u);
    var v3 : vec3<u32> = vec3<u32>(1u + 2u);
    var v4 : vec4<u32> = vec4<u32>(1u + 2u);
}

fn get_bool() -> bool { return true; }
fn get_f32() -> f32 { return 1.0; }
fn get_i32() -> i32 { return 1; }
fn get_u32() -> u32 { return 1u; }

fn from_call_bool() {
    var v2 : vec2<bool> = vec2<bool>(get_bool());
    var v3 : vec3<bool> = vec3<bool>(get_bool());
    var v4 : vec4<bool> = vec4<bool>(get_bool());
}

fn from_call_f32() {
    var v2 : vec2<f32> = vec2<f32>(get_f32());
    var v3 : vec3<f32> = vec3<f32>(get_f32());
    var v4 : vec4<f32> = vec4<f32>(get_f32());
}

fn from_call_i32() {
    var v2 : vec2<i32> = vec2<i32>(get_i32());
    var v3 : vec3<i32> = vec3<i32>(get_i32());
    var v4 : vec4<i32> = vec4<i32>(get_i32());
}

fn from_call_u32() {
    var v2 : vec2<u32> = vec2<u32>(get_u32());
    var v3 : vec3<u32> = vec3<u32>(get_u32());
    var v4 : vec4<u32> = vec4<u32>(get_u32());
}

fn with_swizzle() {
    var a = vec2<f32>(1.0).y;
    var b = vec3<f32>(1.0).z;
    var c = vec4<f32>(1.0).w;
}

[[stage(fragment)]]
fn main() -> [[location(0)]] vec4<f32> {
    return vec4<f32>(0.0,0.0,0.0,0.0);
}
