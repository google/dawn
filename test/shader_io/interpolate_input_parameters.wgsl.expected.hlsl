struct tint_symbol_1 {
  float none : TEXCOORD0;
  nointerpolation float flat : TEXCOORD1;
  linear float perspective_center : TEXCOORD2;
  linear centroid float perspective_centroid : TEXCOORD3;
  linear sample float perspective_sample : TEXCOORD4;
  noperspective float linear_center : TEXCOORD5;
  noperspective centroid float linear_centroid : TEXCOORD6;
  noperspective sample float linear_sample : TEXCOORD7;
};

void main(tint_symbol_1 tint_symbol) {
  const float none = tint_symbol.none;
  const float flat = tint_symbol.flat;
  const float perspective_center = tint_symbol.perspective_center;
  const float perspective_centroid = tint_symbol.perspective_centroid;
  const float perspective_sample = tint_symbol.perspective_sample;
  const float linear_center = tint_symbol.linear_center;
  const float linear_centroid = tint_symbol.linear_centroid;
  const float linear_sample = tint_symbol.linear_sample;
  return;
}
