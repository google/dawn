vk-gl-cts/rasterization/line_continuity/line-strip/1.wgsl:9:26 warning: use of deprecated intrinsic
  let x_22 : vec4<f32> = textureLoad(texture, vec2<i32>(vec2<f32>(x_19.x, x_19.y)));
                         ^^^^^^^^^^^

static float4 color_out = float4(0.0f, 0.0f, 0.0f, 0.0f);
Texture2D<float4> tint_symbol : register(t0, space0);
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  const float4 x_19 = gl_FragCoord;
  const float4 x_22 = tint_symbol.Load(int3(int2(float2(x_19.x, x_19.y)), 0));
  color_out = x_22;
  return;
}

struct main_out {
  float4 color_out_1;
};
struct tint_symbol_2 {
  float4 gl_FragCoord_param : SV_Position;
};
struct tint_symbol_3 {
  float4 color_out_1 : SV_Target0;
};

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  const main_out tint_symbol_4 = {color_out};
  return tint_symbol_4;
}

tint_symbol_3 main(tint_symbol_2 tint_symbol_1) {
  const main_out inner_result = main_inner(tint_symbol_1.gl_FragCoord_param);
  tint_symbol_3 wrapper_result = (tint_symbol_3)0;
  wrapper_result.color_out_1 = inner_result.color_out_1;
  return wrapper_result;
}
