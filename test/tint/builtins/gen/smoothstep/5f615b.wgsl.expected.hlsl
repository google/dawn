builtins/gen/smoothstep/5f615b.wgsl:28:24 warning: use of deprecated builtin
  var res: vec4<f32> = smoothStep(vec4<f32>(), vec4<f32>(), vec4<f32>());
                       ^^^^^^^^^^

void smoothStep_5f615b() {
  float4 res = smoothstep(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  smoothStep_5f615b();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  smoothStep_5f615b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  smoothStep_5f615b();
  return;
}
