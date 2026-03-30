#version 460 core

layout(local_size_x = X_INVOCATIONS, local_size_y = 1, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D screen;
layout(rgba32f, binding = 1) uniform image2D fft_data;

layout(std430, binding = 2) buffer Data{

    uint num_bits;
    int forward;
    int vertical;
    float norm;

};

#define M_PI 3.14159265358979323846212833832795

shared vec2 input_b[NUM_SAMPLES];
shared vec2 real_imag_buffer[NUM_SAMPLES];

shared vec4 transpose_a[NUM_SAMPLES];
shared vec4 transpose_b[NUM_SAMPLES];

int num_samples_h = NUM_SAMPLES ;


//-------------------------------------------------------------------------COMPLEX MULT

vec2 mult(vec2 a, vec2 b){
    
    vec2 result;

    result.x = a.x * b.x - a.y * b.y;
    result.y = a.y * b.x + a.x * b.y;

    return result;

}


//------------------------------------------------------------------------REVERSE BITS
uint rev(uint n, uint num_bits){

    uint res = 0;
    uint i = 0;

    while(i <= num_bits - 1){
    
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

     memoryBarrierShared();
       barrier();
      
}
//-------------------------------------------------------------------------LOAD PIXELS FROM IMAGE TO INPUT

void loadPixs_from_img() {

    ivec2 texCoor = ivec2(gl_GlobalInvocationID.xy);

    for (int i = 0; i < num_samples_h / gl_WorkGroupSize.x ; i++) {

    float x_x = imageLoad(screen, ivec2(texCoor.x + gl_WorkGroupSize.x * i, texCoor.y)).x;
    float y = 0.0;

        if(forward > 0){

            y = imageLoad(screen, ivec2(texCoor.x + gl_WorkGroupSize.x * i, texCoor.y)).y;

        }

        if(vertical == 1){

            y = imageLoad(screen, ivec2(texCoor.x + gl_WorkGroupSize.x * i, texCoor.y)).y;

        }

        if(forward == -1 && vertical == 0){
            //NOOOOOO DDDDEEEEEEEMMM GAMMA CORRECTION!!
            x_x = pow(imageLoad(screen, ivec2(texCoor.x + gl_WorkGroupSize.x * i, texCoor.y)).x, 1.0 / 2.2);

        }

        vec2 cmplx_term = vec2(x_x, y);
        input_b[texCoor.x + gl_WorkGroupSize.x * i] = cmplx_term;

        synchronize();

    }
}

//-------------------------------------------------------------------------LOAD PIXELS FROM BUFFER TO INPUT

void loadPixs_buffer_to_input_b(){

    ivec2 texCoor = ivec2(gl_GlobalInvocationID.xy);

    for (int i = 0; i < num_samples_h / gl_WorkGroupSize.x; i++){

        input_b[texCoor.x + gl_WorkGroupSize.x * i] = real_imag_buffer[texCoor.x + gl_WorkGroupSize.x * i];

        synchronize();

    }

}

//-------------------------------------------------------------------------LOAD PIXELS FROM BUFFER TO INPUT

void loadPixs_input_b_to_buffer(){

    ivec2 texCoor = ivec2(gl_GlobalInvocationID.xy);


    for (int i = 0; i < num_samples_h / gl_WorkGroupSize.x; i++){

        real_imag_buffer[texCoor.x + gl_WorkGroupSize.x * i] = input_b[texCoor.x + gl_WorkGroupSize.x * i];

        synchronize();

    }

}

//-------------------------------------------------------------------------PERMUTATE
void permutate(){
    
    uint num = 1;
    uint bits = 0;

    while(num < num_samples_h){
        
        num <<= 1;
        bits++;
    
    }
    
    ivec2 texCoor = ivec2(gl_GlobalInvocationID.xy);
    uint t_id = texCoor.x;

    for (int i = 0; i < num_samples_h / gl_WorkGroupSize.x; i++){

        uint pair = t_id + gl_WorkGroupSize.x * i;
        real_imag_buffer[pair] = input_b[rev(pair, num_bits)];

        synchronize();

    }



}





void fft(){

    ivec2 texC_g = ivec2(gl_GlobalInvocationID.xy);

    uint t_id = texC_g.x;

    uint k = 2;
    uint num_lvls = uint(log2(num_samples_h));

    uint bttrfls_per_thrd = (num_samples_h / 2) / gl_WorkGroupSize.x;

    for (uint lvl = 0; lvl < num_lvls; lvl++){

        vec4 v = vec4(0.0);
        vec4 v_1 = vec4(0.0);

        for (int b = 0; b < bttrfls_per_thrd; b++){

            float angle = 2.0 * M_PI * float((t_id * bttrfls_per_thrd + b) % (k / 2)) / float(k);

            vec2 twiddle = vec2(cos(angle), forward * sin(angle));

            uint block = (t_id * bttrfls_per_thrd + b) / (k / 2);
            uint offset = (t_id * bttrfls_per_thrd + b) % (k / 2);

            uint e = block * k + offset;
            uint o = e + (k / 2);
            vec2 even = input_b[e];
            vec2 odd = input_b[o];

            vec2 freq = even + mult(odd, twiddle);
            vec2 freq_alias = even - mult(odd, twiddle);

            v = vec4(freq.x, freq.y, 0.0, 1.0);
            v_1 = vec4(freq_alias.x, freq_alias.y, 0.0, 1.0);

            real_imag_buffer[e] = vec2(v.x, v.y);
            real_imag_buffer[o] = vec2(v_1.x, v_1.y);

            input_b[o] = real_imag_buffer[o];
            input_b[e] = real_imag_buffer[e];

        }

        k *= 2;

        synchronize();

   }

}





void main(){

    ivec2 texC_g = ivec2(gl_GlobalInvocationID.xy);

    loadPixs_from_img();

    permutate();
    loadPixs_buffer_to_input_b();
    fft();


    for (int i = 0; i < num_samples_h / gl_WorkGroupSize.x; i++){

        vec2 v = vec2(input_b[texC_g.x + gl_WorkGroupSize.x * i].x, input_b[texC_g.x + gl_WorkGroupSize.x * i].y);

        vec4 v_out = vec4(v.x, v.y, 0.0, 1.0);
        if(norm == (255.0 * 255.0)){

            v_out = vec4(v.x/norm, v.x/norm, v.x/norm, 1.0);

        }
        imageStore(fft_data, ivec2(texC_g.x + gl_WorkGroupSize.x * i, texC_g.y), v_out);

        

    }





}
