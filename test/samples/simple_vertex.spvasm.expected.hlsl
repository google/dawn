struct main_out {
  float4 gl_Position : SV_Position;
};

main_out main() {
  main_out tint_out = (main_out)0;
  tint_out.gl_Position = float4(0.0f, 0.0f, 0.0f, 0.0f);
  return tint_out;
}

