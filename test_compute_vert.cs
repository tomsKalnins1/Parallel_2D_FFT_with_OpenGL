#version 430 core
layout(local_size_x = 1, local_size_y = 256, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D fft_data; //input for vertical fft in this shader
layout(rgba32f, binding = 1) uniform image2D output_1;

#define M_PI 3.1415926535897932384626433832795

shared vec2 real_imag[256];
shared vec2 real_imag_buffer[256];
shared vec4 cache[256];
shared vec4 real[256];
shared vec4 imag[256];





//-------------------------------------------------------------------------COMPLEX ADD_1
vec2 add_1(vec2 a, vec2 b){
    
    vec2 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;

    return result;

}

//-------------------------------------------------------------------------COMPLEX MULT_1

vec2 mult_1(vec2 a, vec2 b){
    
    vec2 result;

    result.x = a.x * b.x - a.y * b.y;
    result.y = a.y * b.x + a.x * b.y;

    return result;

}

//------------------------------------------------------------------------REVERSE BITS
uint rev(uint n, uint num_bits){

    uint res = 0;

    uint i = 0;

    while(i <= num_bits - 1 ){
    
        res <<= 1;

        if((n & 1) == 1){
            
            res |= 1;

        }

        n >>= 1;

        i++;

    }

    return res;
}




//-------------------------------------------------------------------------SYNCHRONIZE DATA ACCESS
 void synchronize(){
    
       barrier();
       memoryBarrierShared();
}
//-------------------------------------------------------------------------PERMUTATE


void permutate1(){
    
    uint num = 1;
    uint bits = 0;

    //this is repetative calculation !! later set the num of bytes as uniform or somoething

    while(num <= gl_WorkGroupSize.y - 1){
        
        num <<= 1;
        bits++;
    
    }
        ivec2 texCoor = ivec2(gl_GlobalInvocationID.xy);

        vec4  temp = imageLoad(fft_data, texCoor);


        real_imag[rev(texCoor.y, bits)] = vec2(temp.x, 0.0);


    
        synchronize();

}





void fft(){

    ivec2 texC_g = ivec2(gl_GlobalInvocationID.xy);

    vec4 v = vec4(0.0);
    vec4 v_1 = vec4(0.0);
    vec2 freq = vec2(0.0);
    vec2 freq_1 = vec2(0.0);


    uint k = 2;
    uint num_lvls = uint(log2(gl_WorkGroupSize.y / 2));

    for(uint lvl = 0; lvl < num_lvls; lvl++){


            float angle = 2.0 * M_PI * float((gl_LocalInvocationIndex) % (k/2)) / float(k);

            vec2 twiddle = vec2(cos(angle), -sin(angle)); 


            uint block = gl_LocalInvocationIndex / (k / 2);
            uint offset = gl_LocalInvocationIndex % (k / 2);

            uint e = block * k + offset;
            uint o = e + (k / 2);


            vec2 even = real_imag[e];

            vec2 odd = real_imag[o];
            vec2 freq = even + mult_1(odd, twiddle);
            vec2 freq_alias = even - mult_1(odd, twiddle);

            v = vec4(freq.x, freq.y, 0.0, 1.0);

            v_1 = vec4(freq_alias.x, freq_alias.y, 0.0, 1.0);

            real_imag_buffer[e] = vec2(v.x, v.y);

            real_imag_buffer[o] = vec2(v_1.x, v_1.y);

            synchronize();

            real_imag[o] = real_imag_buffer[o];



            real_imag[e] = real_imag_buffer[e];



            synchronize();


        k <<= 1;
        synchronize();

    }



}





void main()
{
  
    
 
    permutate1();

 

    fft();





    ivec2 texC_g = ivec2(gl_GlobalInvocationID.xy);

    vec2 fft_res = real_imag[gl_LocalInvocationIndex];
    


    vec4 v = vec4(fft_res.x, fft_res.y , 0.0, 1.0);


    imageStore(output_1, texC_g, v);




}
