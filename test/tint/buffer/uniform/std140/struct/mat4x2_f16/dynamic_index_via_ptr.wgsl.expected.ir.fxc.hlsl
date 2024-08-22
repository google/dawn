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

Failed to generate: :60:5 error: binary: no matching overload for 'operator * (i32, u32)'

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

    %48:u32 = mul %47, 2u
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
    %20:Outer = call %21, %10
    %l_a_i:Outer = let %20
    %23:array<Inner, 4> = call %24, %10
    %l_a_i_a:array<Inner, 4> = let %23
    %26:u32 = add %10, %13
    %27:Inner = call %28, %26
    %l_a_i_a_i:Inner = let %27
    %30:u32 = add %10, %13
    %31:mat4x2<f16> = call %32, %30
    %l_a_i_a_i_m:mat4x2<f16> = let %31
    %34:u32 = add %10, %13
    %35:u32 = add %34, %16
    %36:u32 = div %35, 16u
    %37:ptr<uniform, vec4<u32>, read> = access %a, %36
    %38:u32 = mod %35, 16u
    %39:u32 = div %38, 4u
    %40:vec4<u32> = load %37
    %41:u32 = swizzle %40, z
    %42:u32 = swizzle %40, x
    %43:bool = eq %39, 2u
    %44:u32 = hlsl.ternary %42, %41, %43
    %45:vec2<f16> = bitcast %44
    %l_a_i_a_i_m_i:vec2<f16> = let %45
    %47:i32 = call %i
    %48:u32 = mul %47, 2u
    %49:u32 = add %10, %13
    %50:u32 = add %49, %16
    %51:u32 = add %50, %48
    %52:u32 = div %51, 16u
    %53:ptr<uniform, vec4<u32>, read> = access %a, %52
    %54:u32 = mod %51, 16u
    %55:u32 = div %54, 4u
    %56:u32 = load_vector_element %53, %55
    %57:u32 = mod %51, 4u
    %58:bool = eq %57, 0u
    %59:u32 = hlsl.ternary 16u, 0u, %58
    %60:u32 = shr %56, %59
    %61:f32 = hlsl.f16tof32 %60
    %62:f16 = convert %61
    %l_a_i_a_i_m_i_i:f16 = let %62
    ret
  }
}
%32 = func(%start_byte_offset:u32):mat4x2<f16> {
  $B4: {
    %65:u32 = div %start_byte_offset, 16u
    %66:ptr<uniform, vec4<u32>, read> = access %a, %65
    %67:u32 = mod %start_byte_offset, 16u
    %68:u32 = div %67, 4u
    %69:vec4<u32> = load %66
    %70:u32 = swizzle %69, z
    %71:u32 = swizzle %69, x
    %72:bool = eq %68, 2u
    %73:u32 = hlsl.ternary %71, %70, %72
    %74:vec2<f16> = bitcast %73
    %75:u32 = add 4u, %start_byte_offset
    %76:u32 = div %75, 16u
    %77:ptr<uniform, vec4<u32>, read> = access %a, %76
    %78:u32 = mod %75, 16u
    %79:u32 = div %78, 4u
    %80:vec4<u32> = load %77
    %81:u32 = swizzle %80, z
    %82:u32 = swizzle %80, x
    %83:bool = eq %79, 2u
    %84:u32 = hlsl.ternary %82, %81, %83
    %85:vec2<f16> = bitcast %84
    %86:u32 = add 8u, %start_byte_offset
    %87:u32 = div %86, 16u
    %88:ptr<uniform, vec4<u32>, read> = access %a, %87
    %89:u32 = mod %86, 16u
    %90:u32 = div %89, 4u
    %91:vec4<u32> = load %88
    %92:u32 = swizzle %91, z
    %93:u32 = swizzle %91, x
    %94:bool = eq %90, 2u
    %95:u32 = hlsl.ternary %93, %92, %94
    %96:vec2<f16> = bitcast %95
    %97:u32 = add 12u, %start_byte_offset
    %98:u32 = div %97, 16u
    %99:ptr<uniform, vec4<u32>, read> = access %a, %98
    %100:u32 = mod %97, 16u
    %101:u32 = div %100, 4u
    %102:vec4<u32> = load %99
    %103:u32 = swizzle %102, z
    %104:u32 = swizzle %102, x
    %105:bool = eq %101, 2u
    %106:u32 = hlsl.ternary %104, %103, %105
    %107:vec2<f16> = bitcast %106
    %108:mat4x2<f16> = construct %74, %85, %96, %107
    ret %108
  }
}
%28 = func(%start_byte_offset_1:u32):Inner {  # %start_byte_offset_1: 'start_byte_offset'
  $B5: {
    %110:mat4x2<f16> = call %32, %start_byte_offset_1
    %111:Inner = construct %110
    ret %111
  }
}
%24 = func(%start_byte_offset_2:u32):array<Inner, 4> {  # %start_byte_offset_2: 'start_byte_offset'
  $B6: {
    %a_1:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat4x2<f16>(vec2<f16>(0.0h))))  # %a_1: 'a'
    loop [i: $B7, b: $B8, c: $B9] {  # loop_1
      $B7: {  # initializer
        next_iteration 0u  # -> $B8
      }
      $B8 (%idx:u32): {  # body
        %115:bool = gte %idx, 4u
        if %115 [t: $B10] {  # if_1
          $B10: {  # true
            exit_loop  # loop_1
          }
        }
        %116:u32 = mul %idx, 64u
        %117:u32 = add %start_byte_offset_2, %116
        %118:ptr<function, Inner, read_write> = access %a_1, %idx
        %119:Inner = call %28, %117
        store %118, %119
        continue  # -> $B9
      }
      $B9: {  # continuing
        %120:u32 = add %idx, 1u
        next_iteration %120  # -> $B8
      }
    }
    %121:array<Inner, 4> = load %a_1
    ret %121
  }
}
%21 = func(%start_byte_offset_3:u32):Outer {  # %start_byte_offset_3: 'start_byte_offset'
  $B11: {
    %123:array<Inner, 4> = call %24, %start_byte_offset_3
    %124:Outer = construct %123
    ret %124
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
        %128:bool = gte %idx_1, 4u
        if %128 [t: $B16] {  # if_2
          $B16: {  # true
            exit_loop  # loop_2
          }
        }
        %129:u32 = mul %idx_1, 256u
        %130:u32 = add %start_byte_offset_4, %129
        %131:ptr<function, Outer, read_write> = access %a_2, %idx_1
        %132:Outer = call %21, %130
        store %131, %132
        continue  # -> $B15
      }
      $B15: {  # continuing
        %133:u32 = add %idx_1, 1u
        next_iteration %133  # -> $B14
      }
    }
    %134:array<Outer, 4> = load %a_2
    ret %134
  }
}

