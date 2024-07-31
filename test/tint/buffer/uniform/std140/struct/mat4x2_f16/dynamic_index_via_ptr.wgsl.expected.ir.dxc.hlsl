SKIP: FAILED


enable f16;

struct Inner {
  @size(64)
  m : mat4x2<f16>,
}

struct Outer {
  a : array<Inner, 4>,
}

@group(0) @binding(0) var<uniform> a : array<Outer, 4>;

var<private> counter = 0;

fn i() -> i32 {
  counter++;
  return counter;
}

@compute @workgroup_size(1)
fn f() {
  let p_a = &(a);
  let p_a_i = &((*(p_a))[i()]);
  let p_a_i_a = &((*(p_a_i)).a);
  let p_a_i_a_i = &((*(p_a_i_a))[i()]);
  let p_a_i_a_i_m = &((*(p_a_i_a_i)).m);
  let p_a_i_a_i_m_i = &((*(p_a_i_a_i_m))[i()]);
  let l_a : array<Outer, 4> = *(p_a);
  let l_a_i : Outer = *(p_a_i);
  let l_a_i_a : array<Inner, 4> = *(p_a_i_a);
  let l_a_i_a_i : Inner = *(p_a_i_a_i);
  let l_a_i_a_i_m : mat4x2<f16> = *(p_a_i_a_i_m);
  let l_a_i_a_i_m_i : vec2<f16> = *(p_a_i_a_i_m_i);
  let l_a_i_a_i_m_i_i : f16 = (*(p_a_i_a_i_m_i))[i()];
}

Failed to generate: :38:24 error: binary: %23 is not in scope
    %22:u32 = add %21, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:68:5 note: %23 declared here
    %23:u32 = mul %56, 2u
    ^^^^^^^

:43:24 error: binary: %23 is not in scope
    %29:u32 = add %28, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:68:5 note: %23 declared here
    %23:u32 = mul %56, 2u
    ^^^^^^^

:48:24 error: binary: %23 is not in scope
    %35:u32 = add %34, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:68:5 note: %23 declared here
    %23:u32 = mul %56, 2u
    ^^^^^^^

:68:5 error: binary: no matching overload for 'operator * (i32, u32)'

9 candidate operators:
 • 'operator * (T  ✓ , T  ✗ ) -> T' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (vecN<T>  ✗ , T  ✓ ) -> vecN<T>' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (T  ✓ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (T  ✗ , matNxM<T>  ✗ ) -> matNxM<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (matNxM<T>  ✗ , T  ✗ ) -> matNxM<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (vecN<T>  ✗ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (matCxR<T>  ✗ , vecC<T>  ✗ ) -> vecR<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (vecR<T>  ✗ , matCxR<T>  ✗ ) -> vecC<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (matKxR<T>  ✗ , matCxK<T>  ✗ ) -> matCxR<T>' where:
      ✗  'T' is 'f32' or 'f16'

    %23:u32 = mul %56, 2u
    ^^^^^^^^^^^^^^^^^^^^^

:24:3 note: in block
  $B3: {
  ^^^

note: # Disassembly
Inner = struct @align(4) {
  m:mat4x2<f16> @offset(0)
}

Outer = struct @align(4) {
  a:array<Inner, 4> @offset(0)
}

$B1: {  # root
  %a:ptr<uniform, array<vec4<u32>, 64>, read> = var @binding_point(0, 0)
  %counter:ptr<private, i32, read_write> = var, 0i
}

%i = func():i32 {
  $B2: {
    %4:i32 = load %counter
    %5:i32 = add %4, 1i
    store %counter, %5
    %6:i32 = load %counter
    ret %6
  }
}
%f = @compute @workgroup_size(1, 1, 1) func():void {
  $B3: {
    %8:i32 = call %i
    %9:u32 = convert %8
    %10:u32 = mul 256u, %9
    %11:i32 = call %i
    %12:u32 = convert %11
    %13:u32 = mul 64u, %12
    %14:i32 = call %i
    %15:u32 = convert %14
    %16:u32 = mul 4u, %15
    %17:array<Outer, 4> = call %18, 0u
    %l_a:array<Outer, 4> = let %17
    %20:u32 = add %10, %13
    %21:u32 = add %20, %16
    %22:u32 = add %21, %23
    %24:Outer = call %25, %22
    %l_a_i:Outer = let %24
    %27:u32 = add %10, %13
    %28:u32 = add %27, %16
    %29:u32 = add %28, %23
    %30:array<Inner, 4> = call %31, %29
    %l_a_i_a:array<Inner, 4> = let %30
    %33:u32 = add %10, %13
    %34:u32 = add %33, %16
    %35:u32 = add %34, %23
    %36:Inner = call %37, %35
    %l_a_i_a_i:Inner = let %36
    %39:u32 = add %10, %13
    %40:mat4x2<f16> = call %41, %39
    %l_a_i_a_i_m:mat4x2<f16> = let %40
    %43:u32 = add %10, %13
    %44:u32 = add %43, %16
    %45:u32 = div %44, 16u
    %46:ptr<uniform, vec4<u32>, read> = access %a, %45
    %47:u32 = mod %44, 16u
    %48:u32 = div %47, 4u
    %49:vec4<u32> = load %46
    %50:u32 = swizzle %49, z
    %51:u32 = swizzle %49, x
    %52:bool = eq %48, 2u
    %53:u32 = hlsl.ternary %51, %50, %52
    %54:vec2<f16> = bitcast %53
    %l_a_i_a_i_m_i:vec2<f16> = let %54
    %56:i32 = call %i
    %23:u32 = mul %56, 2u
    %57:u32 = add %10, %13
    %58:u32 = add %57, %16
    %59:u32 = add %58, %23
    %60:u32 = div %59, 16u
    %61:ptr<uniform, vec4<u32>, read> = access %a, %60
    %62:u32 = mod %59, 16u
    %63:u32 = div %62, 4u
    %64:u32 = load_vector_element %61, %63
    %65:u32 = mod %59, 4u
    %66:bool = eq %65, 0u
    %67:u32 = hlsl.ternary 16u, 0u, %66
    %68:u32 = shr %64, %67
    %69:f32 = hlsl.f16tof32 %68
    %70:f16 = convert %69
    %l_a_i_a_i_m_i_i:f16 = let %70
    ret
  }
}
%41 = func(%start_byte_offset:u32):mat4x2<f16> {
  $B4: {
    %73:u32 = div %start_byte_offset, 16u
    %74:ptr<uniform, vec4<u32>, read> = access %a, %73
    %75:u32 = mod %start_byte_offset, 16u
    %76:u32 = div %75, 4u
    %77:vec4<u32> = load %74
    %78:u32 = swizzle %77, z
    %79:u32 = swizzle %77, x
    %80:bool = eq %76, 2u
    %81:u32 = hlsl.ternary %79, %78, %80
    %82:vec2<f16> = bitcast %81
    %83:u32 = add 4u, %start_byte_offset
    %84:u32 = div %83, 16u
    %85:ptr<uniform, vec4<u32>, read> = access %a, %84
    %86:u32 = mod %83, 16u
    %87:u32 = div %86, 4u
    %88:vec4<u32> = load %85
    %89:u32 = swizzle %88, z
    %90:u32 = swizzle %88, x
    %91:bool = eq %87, 2u
    %92:u32 = hlsl.ternary %90, %89, %91
    %93:vec2<f16> = bitcast %92
    %94:u32 = add 8u, %start_byte_offset
    %95:u32 = div %94, 16u
    %96:ptr<uniform, vec4<u32>, read> = access %a, %95
    %97:u32 = mod %94, 16u
    %98:u32 = div %97, 4u
    %99:vec4<u32> = load %96
    %100:u32 = swizzle %99, z
    %101:u32 = swizzle %99, x
    %102:bool = eq %98, 2u
    %103:u32 = hlsl.ternary %101, %100, %102
    %104:vec2<f16> = bitcast %103
    %105:u32 = add 12u, %start_byte_offset
    %106:u32 = div %105, 16u
    %107:ptr<uniform, vec4<u32>, read> = access %a, %106
    %108:u32 = mod %105, 16u
    %109:u32 = div %108, 4u
    %110:vec4<u32> = load %107
    %111:u32 = swizzle %110, z
    %112:u32 = swizzle %110, x
    %113:bool = eq %109, 2u
    %114:u32 = hlsl.ternary %112, %111, %113
    %115:vec2<f16> = bitcast %114
    %116:mat4x2<f16> = construct %82, %93, %104, %115
    ret %116
  }
}
%37 = func(%start_byte_offset_1:u32):Inner {  # %start_byte_offset_1: 'start_byte_offset'
  $B5: {
    %118:mat4x2<f16> = call %41, %start_byte_offset_1
    %119:Inner = construct %118
    ret %119
  }
}
%31 = func(%start_byte_offset_2:u32):array<Inner, 4> {  # %start_byte_offset_2: 'start_byte_offset'
  $B6: {
    %a_1:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat4x2<f16>(vec2<f16>(0.0h))))  # %a_1: 'a'
    loop [i: $B7, b: $B8, c: $B9] {  # loop_1
      $B7: {  # initializer
        next_iteration 0u  # -> $B8
      }
      $B8 (%idx:u32): {  # body
        %123:bool = gte %idx, 4u
        if %123 [t: $B10] {  # if_1
          $B10: {  # true
            exit_loop  # loop_1
          }
        }
        %124:u32 = mul %idx, 64u
        %125:u32 = add %start_byte_offset_2, %124
        %126:ptr<function, Inner, read_write> = access %a_1, %idx
        %127:Inner = call %37, %125
        store %126, %127
        continue  # -> $B9
      }
      $B9: {  # continuing
        %128:u32 = add %idx, 1u
        next_iteration %128  # -> $B8
      }
    }
    %129:array<Inner, 4> = load %a_1
    ret %129
  }
}
%25 = func(%start_byte_offset_3:u32):Outer {  # %start_byte_offset_3: 'start_byte_offset'
  $B11: {
    %131:array<Inner, 4> = call %31, %start_byte_offset_3
    %132:Outer = construct %131
    ret %132
  }
}
%18 = func(%start_byte_offset_4:u32):array<Outer, 4> {  # %start_byte_offset_4: 'start_byte_offset'
  $B12: {
    %a_2:ptr<function, array<Outer, 4>, read_write> = var, array<Outer, 4>(Outer(array<Inner, 4>(Inner(mat4x2<f16>(vec2<f16>(0.0h))))))  # %a_2: 'a'
    loop [i: $B13, b: $B14, c: $B15] {  # loop_2
      $B13: {  # initializer
        next_iteration 0u  # -> $B14
      }
      $B14 (%idx_1:u32): {  # body
        %136:bool = gte %idx_1, 4u
        if %136 [t: $B16] {  # if_2
          $B16: {  # true
            exit_loop  # loop_2
          }
        }
        %137:u32 = mul %idx_1, 256u
        %138:u32 = add %start_byte_offset_4, %137
        %139:ptr<function, Outer, read_write> = access %a_2, %idx_1
        %140:Outer = call %25, %138
        store %139, %140
        continue  # -> $B15
      }
      $B15: {  # continuing
        %141:u32 = add %idx_1, 1u
        next_iteration %141  # -> $B14
      }
    }
    %142:array<Outer, 4> = load %a_2
    ret %142
  }
}

