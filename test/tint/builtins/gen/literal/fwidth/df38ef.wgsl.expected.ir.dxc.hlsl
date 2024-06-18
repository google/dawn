SKIP: FAILED

float fwidth_df38ef() {
  float res = fwidth(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce = fwidth_df38ef();
}

DXC validation failure:
hlsl.hlsl:7:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = fwidth_df38ef();
  ^

