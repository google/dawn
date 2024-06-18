SKIP: FAILED

[numthreads(1, 1, 1)]
void f() {
  int l1 = v1;
  uint l2 = v2;
  float l3 = v3;
  int3 l4 = v4;
  uint3 l5 = v5;
  float3 l6 = v6;
  MyStruct l7 = v7;
  float[10] l8 = v8;
  int l9 = v9;
  uint l10 = v10;
  float l11 = v11;
  MyStruct l12 = v12;
  MyStruct l13 = v13;
  float[10] l14 = v14;
  int3 l15 = v15;
  float3 l16 = v16;
}

DXC validation failure:
hlsl.hlsl:3:12: error: use of undeclared identifier 'v1'
  int l1 = v1;
           ^
hlsl.hlsl:4:13: error: use of undeclared identifier 'v2'
  uint l2 = v2;
            ^
hlsl.hlsl:5:14: error: use of undeclared identifier 'v3'
  float l3 = v3;
             ^
hlsl.hlsl:6:13: error: use of undeclared identifier 'v4'
  int3 l4 = v4;
            ^
hlsl.hlsl:7:14: error: use of undeclared identifier 'v5'
  uint3 l5 = v5;
             ^
hlsl.hlsl:8:15: error: use of undeclared identifier 'v6'
  float3 l6 = v6;
              ^
hlsl.hlsl:9:3: error: unknown type name 'MyStruct'
  MyStruct l7 = v7;
  ^
hlsl.hlsl:10:15: error: brackets are not allowed here; to declare an array, place the brackets after the name
  float[10] l8 = v8;
       ~~~~   ^
              [10]
hlsl.hlsl:10:18: error: use of undeclared identifier 'v8'
  float[10] l8 = v8;
                 ^
hlsl.hlsl:11:12: error: use of undeclared identifier 'v9'
  int l9 = v9;
           ^
hlsl.hlsl:12:14: error: use of undeclared identifier 'v10'
  uint l10 = v10;
             ^
hlsl.hlsl:13:15: error: use of undeclared identifier 'v11'
  float l11 = v11;
              ^
hlsl.hlsl:14:3: error: unknown type name 'MyStruct'
  MyStruct l12 = v12;
  ^
hlsl.hlsl:15:3: error: unknown type name 'MyStruct'
  MyStruct l13 = v13;
  ^
hlsl.hlsl:16:16: error: brackets are not allowed here; to declare an array, place the brackets after the name
  float[10] l14 = v14;
       ~~~~    ^
               [10]
hlsl.hlsl:16:19: error: use of undeclared identifier 'v14'
  float[10] l14 = v14;
                  ^
hlsl.hlsl:17:14: error: use of undeclared identifier 'v15'
  int3 l15 = v15;
             ^
hlsl.hlsl:18:16: error: use of undeclared identifier 'v16'
  float3 l16 = v16;
               ^

