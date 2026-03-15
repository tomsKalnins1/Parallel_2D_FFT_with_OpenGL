#version 430 core
layout(local_size_x = 1, local_size_y = 128, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D fft_data; //input for vertical fft in this shader
layout(rgba32f, binding = 1) uniform image2D output_1;

#define M_PI 3.1415926535897932384626433832795

shared vec2 input[256];
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

//-------------------------------------------------------------------------COMPLEX SUB_1

vec2 sub_1(vec2 a, vec2 b)
{

    vec2 result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;

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
void synchronize()
{

    barrier();
    memoryBarrierShared();
}

//-------------------------------------------------------------------------LOAD PIXELS FROM IMAGE TO INPUT

void loadPixs_from_img()
{

    ivec2 texCoor = ivec2(gl_GlobalInvocationID.xy);

    vec2 val_0 = vec2(imageLoad(fft_data, texCoor).xy);
    vec2 val_1 = vec2(imageLoad(fft_data, ivec2(texCoor.x, texCoor.y + 128)).xy);

    input[texCoor.y] = val_0;

    synchronize();

    input[texCoor.y + 128] = val_1;

    synchronize();

}

//-------------------------------------------------------------------------LOAD PIXELS FROM BUFFER TO INPUT

void loadPixs_buffer_to_input()
{

    ivec2 texCoor = ivec2(gl_GlobalInvocationID.xy);



    input[texCoor.y] = real_imag_buffer[texCoor.y];


    synchronize();

    input[texCoor.y + 128] = real_imag_buffer[texCoor.y + 128];


    synchronize();
}

//-------------------------------------------------------------------------LOAD PIXELS FROM BUFFER TO INPUT

void loadPixs_input_to_buffer()
{

    ivec2 texCoor = ivec2(gl_GlobalInvocationID.xy);



    real_imag_buffer[texCoor.y] = input[texCoor.y];


    synchronize();

    real_imag_buffer[texCoor.y + 128] = input[texCoor.y + 128];


    synchronize();
}



//-------------------------------------------------------------------------PERMUTATE


void permutate1()
{

    uint num = 1;
    uint bits = 0;

    //this is repetative calculation !! later set the num of bytes as uniform or somoething

    while (num < 256)
    {

        num <<= 1;
        bits++;

    }
    ivec2 texCoor = ivec2(gl_GlobalInvocationID.xy);

    uint t_id = texCoor.y;

    real_imag_buffer[t_id] = input[rev(t_id, bits)];

    synchronize();

    uint pair = t_id + 128;

    real_imag_buffer[pair] = input[rev(pair, bits)];

    synchronize();

}






void fft(){

    ivec2 texC_g = ivec2(gl_GlobalInvocationID.xy);


    uint k = 2;
    uint num_lvls = uint(log2(256));

    for(uint lvl = 0; lvl < num_lvls; lvl++){


            vec4 v = vec4(0.0);
            vec4 v_1 = vec4(0.0);

            float angle = 2.0 * M_PI * float((gl_LocalInvocationIndex) % (k/2)) / float(k);

            vec2 twiddle = vec2(cos(angle), -sin(angle)); 


            uint block = gl_LocalInvocationIndex / (k / 2);
            uint offset = gl_LocalInvocationIndex % (k / 2);

            uint e = block * k + offset;
            uint o = e + (k / 2);


            vec2 even = input[e];

            vec2 odd = input[o];
            vec2 freq = even + mult_1(odd, twiddle);
            vec2 freq_alias = even - mult_1(odd, twiddle);

            v = vec4(freq.x, freq.y, 0.0, 1.0);

            v_1 = vec4(freq_alias.x, freq_alias.y, 0.0, 1.0);

            real_imag_buffer[e] = vec2(v.x, v.y);

            real_imag_buffer[o] = vec2(v_1.x, v_1.y);

            synchronize();

            input[o] = real_imag_buffer[o];



            input[e] = real_imag_buffer[e];



            synchronize();


        k *= 2;
        synchronize();

    }



}





void main()
{


    loadPixs_from_img();

    permutate1();
    loadPixs_buffer_to_input();
    fft();
    // loadPixs_input_to_buffer();




    ivec2 texC_g = ivec2(gl_GlobalInvocationID.xy);

    vec2 v_0 = vec2(input[texC_g.y].x, input[texC_g.y].y);

    vec2 v_1 = vec2(input[texC_g.y + 128].x, input[texC_g.y + 128].y);

    vec4 v_0_0 = vec4(v_0.x, v_0.y, 0.0, 0.0);
    vec4 v_1_1 = vec4(v_1.x, v_1.y, 0.0, 0.0);

    imageStore(output_1, texC_g, v_0_0);
    imageStore(output_1, ivec2(texC_g.x, texC_g.y + 128), v_1_1);




}
