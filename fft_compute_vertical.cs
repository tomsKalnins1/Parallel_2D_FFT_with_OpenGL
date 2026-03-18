#version 430 core
layout(local_size_x = 1, local_size_y = Y_INVOCATIONS, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D fft_data; //input for vertical fft in this shader
layout(rgba32f, binding = 1) uniform image2D output_1;

#define M_PI 3.1415926535897932384626433832795

uniform int num_samples_v;

shared vec2 input[NUM_SAMPLES];
shared vec2 real_imag_buffer[NUM_SAMPLES];






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

    for (int i = 0; i < num_samples_v / gl_WorkGroupSize.y; i++)
    {

        vec2 val_1 = vec2(imageLoad(fft_data, ivec2(texCoor.x, texCoor.y + 64 * i)).x, imageLoad(fft_data, ivec2(texCoor.x, texCoor.y + 64 * i)).y);

        input[texCoor.y + 64 * i] = val_1;

        synchronize();

    }

}

//-------------------------------------------------------------------------LOAD PIXELS FROM BUFFER TO INPUT

void loadPixs_buffer_to_input()
{

    ivec2 texCoor = ivec2(gl_GlobalInvocationID.xy);




    for (int i = 0; i < num_samples_v / gl_WorkGroupSize.y; i++)
    {

        input[texCoor.y + 64 * i] = real_imag_buffer[texCoor.y + 64 * i];

        synchronize();

    }
}

//-------------------------------------------------------------------------LOAD PIXELS FROM BUFFER TO INPUT

void loadPixs_input_to_buffer(){

    ivec2 texCoor = ivec2(gl_GlobalInvocationID.xy);



    for (int i = 0; i < num_samples_v / gl_WorkGroupSize.y; i++)
    {

        real_imag_buffer[texCoor.y + 64 * i] = input[texCoor.y + 64 * i];

        synchronize();

    }
}



//-------------------------------------------------------------------------PERMUTATE


void permutate(){

    uint num = 1;
    uint bits = 0;

    //this is repetative calculation !! later set the num of bytes as uniform or somoething

    while (num < num_samples_v)
    {

        num <<= 1;
        bits++;

    }
    ivec2 texCoor = ivec2(gl_GlobalInvocationID.xy);

    uint t_id = texCoor.y;

    for (int i = 0; i < num_samples_v / gl_WorkGroupSize.y; i++)
    {

        uint pair = t_id + 64 * i;

        real_imag_buffer[pair] = input[rev(pair, bits)];

        synchronize();

    }

}






void fft(){


    ivec2 texC_g = ivec2(gl_GlobalInvocationID.xy);

    uint k = 2;
    uint t_id = texC_g.y;
    uint num_lvls = uint(log2(num_samples_v));
    uint bttrfls_per_thrd = (num_samples_v / 2) / gl_WorkGroupSize.y;

    for (uint lvl = 0; lvl < num_lvls; lvl++){


            vec4 v = vec4(0.0);
            vec4 v_1 = vec4(0.0);

        for (int b = 0; b < bttrfls_per_thrd; b++)
        {
            float angle = 2.0 * M_PI * float((t_id * bttrfls_per_thrd + b) % (k / 2)) / float(k);

            vec2 twiddle = vec2(cos(angle), -sin(angle));


            uint block = (t_id * bttrfls_per_thrd + b) / (k / 2);
            uint offset = (t_id * bttrfls_per_thrd + b) % (k / 2);

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
        }

        k *= 2;
        synchronize();

    }



}





void main()
{


    loadPixs_from_img();

    permutate();
    loadPixs_buffer_to_input();
    fft();
    // loadPixs_input_to_buffer();




    ivec2 texC_g = ivec2(gl_GlobalInvocationID.xy);

    for (int i = 0; i < num_samples_v / gl_WorkGroupSize.y; i++)
    {

        vec2 v = vec2(input[texC_g.y + 64 * i].x, input[texC_g.y + 64 * i].y);
        vec4 v_out = vec4(v.x, v.y, 0.0, 0.0);
        imageStore(output_1, ivec2(texC_g.x, texC_g.y + 64 * i), v_out);

    }




}
