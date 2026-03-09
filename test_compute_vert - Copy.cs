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

//-------------------------------------------------------------------------STRUCT COMPLEX

struct complex{

    float real;
    float imag;

};
//-------------------------------------------------------------------------COMPLEX MULT
complex mult(complex a, complex b){
    
    complex result;

    result.real = a.real * b.real - a.imag * b.imag;
    result.imag = a.imag * b.real + a.real * b.imag;

    return result;

}
//-------------------------------------------------------------------------COMPLEX ADD
complex add(complex a, complex b){
    
    complex result;

    result.real = a.real + b.real;
    result.imag = a.imag + b.imag;

    return result;

}

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

uint reverseBits(uint num) 
{ 
    uint count = 31; 
    uint reverse_num = num; 
      
    num >>= 1;  
    while(num != 0) 
    { 
       reverse_num <<= 1;        
       reverse_num |= num & 1; 
       num >>= 1; 
       count--; 
    } 
    reverse_num <<= count; 
    return reverse_num; 
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

        cache[rev(texCoor.y, bits)] = temp;

        real_imag[rev(texCoor.y, bits)] = vec2(temp.x, 0.0);


    
        synchronize();

}





void fft(){

    ivec2 texC_g = ivec2(gl_GlobalInvocationID.xy);

    vec4 v = vec4(0.0);
    vec2 freq = vec2(0.0);
    vec2 freq_1 = vec2(0.0);


    uint k = 2;
    uint num_lvls = uint(log2(gl_WorkGroupSize.y));

    for(uint lvl = 0; lvl < num_lvls; lvl++){
    
         float angle = 2.0 * M_PI * float((gl_LocalInvocationIndex ^ k) % k) / float(k);

         vec2 twiddle = vec2(cos(angle), - sin(angle)); //potential problem with -1 ?

         if( (gl_LocalInvocationIndex ^ k) % k < k/2 ){

            vec2 even = real_imag[gl_LocalInvocationIndex];

            vec2 odd = real_imag[gl_LocalInvocationIndex + k/2];

            freq =  even + mult_1( odd, twiddle ) ;
            
            //vec2 freq = even + vec2(odd.x * twiddle.x, odd.y * twiddle.y);

            //real_imag[gl_LocalInvocationIndex] = freq;

            //float mag  = sqrt( pow( freq.x, 2.0 ) + pow(freq.y, 2.0) );

           // v = vec4(mag/256.0, mag/256.0, mag/256.0, 1.0);

            v = vec4(freq.x, freq.y, 0.0, 1.0);

            //   imageStore(output_1, texC_g, v);

           // v = vec4(1.0, 0.0, 0.0, 1.0);



        }
        else{
         
            vec2 even_1 = real_imag[gl_LocalInvocationIndex - k/2];
            
            vec2 odd_1 = real_imag[gl_LocalInvocationIndex];

            freq_1 = even_1 - mult_1( odd_1, twiddle );

          // vec2 freq_1 = even_1  - vec2(odd_1.x * twiddle.x, odd_1.y * twiddle.y);

       //     real_imag[gl_LocalInvocationIndex] = freq_1;

       //     float mag_1  = sqrt( pow( freq_1.x, 2.0 ) + pow(freq_1.y, 2.0) );

            v = vec4(freq_1.x, freq_1.y, 0.0, 1.0);

            //     v = vec4(mag_1/256.0, mag_1/256.0, mag_1/256.0, 1.0);

            //   imageStore(output_1, texC_g, v);

          //  v = vec4(0.0, 1.0, 0.0, 1.0);


        }

        // synchronize();

        k *= 2;
        synchronize();

        real_imag_buffer[gl_LocalInvocationIndex] = vec2(v.x, v.y);


    
        real_imag[gl_LocalInvocationIndex] = real_imag_buffer[gl_LocalInvocationIndex];

      

        synchronize();






    }



}





void main()
{
  
    
    ivec2 texC_g = ivec2(gl_GlobalInvocationID.xy);
    permutate1();
  //  imageStore(screen_out, texC_g, cache[gl_LocalInvocationIndex]);
    
    fft();

    vec2 fft_res = real_imag[gl_LocalInvocationIndex];
    vec4 v = vec4(fft_res.x, fft_res.y , 0.0, 1.0);
    vec4 temp = imageLoad(fft_data, texC_g);
    // v = v + temp;

   ivec2 t = ivec2(texC_g.x, gl_LocalInvocationIndex);
    imageStore(output_1, t, v);

}
