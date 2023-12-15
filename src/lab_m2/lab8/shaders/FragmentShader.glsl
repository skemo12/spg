#version 410

// Input
layout(location = 0) in vec2 texture_coord;

// Uniform properties
uniform sampler2D textureImage;
uniform ivec2 screenSize;
uniform int flipVertical;
uniform int outputMode = 2; // 0: original, 1: grayscale, 2: blur

// Output
layout(location = 0) out vec4 out_color;

// Local variables
vec2 textureCoord = vec2(texture_coord.x, (flipVertical != 0) ? 1 - texture_coord.y : texture_coord.y); // Flip texture


vec4 grayscale()
{
    vec4 color = texture(textureImage, textureCoord);
    float gray = 0.21 * color.r + 0.71 * color.g + 0.07 * color.b;
    return vec4(gray, gray, gray,  0);
}


vec4 blur(int blurRadius)
{
    vec2 texelSize = 1.0f / screenSize;
    vec4 sum = vec4(0);
    for(int i = -blurRadius; i <= blurRadius; i++)
    {
        for(int j = -blurRadius; j <= blurRadius; j++)
        {
            sum += texture(textureImage, textureCoord + vec2(i, j) * texelSize);
        }
    }

    float samples = pow((2 * blurRadius + 1), 2);
    return sum / samples;
}

float sort_and_get_median(float a[25])
{
    int i, j;
    for (i = 0; i < 24; ++i){
        for(j = 0; j < 24 - i; ++j){
	        if (a[j] > a[j + 1]){
			    float aux = a[j];
			    a[j] = a[j + 1];
			    a[j + 1] = aux;
            }
        }
   }

   return a[12];
}

vec4 filtru_median(int blurRadius) {
    vec2 texelSize = 1.0f / screenSize;
    int index = 0;
    float arr_r[25], arr_g[25], arr_b[25];


    for(int i = -blurRadius; i <= blurRadius; i++)
    {
        for(int j = -blurRadius; j <= blurRadius; j++)
        {
            arr_r[index] = texture(textureImage, textureCoord + vec2(i, j) * texelSize).x;
            arr_g[index] = texture(textureImage, textureCoord + vec2(i, j) * texelSize).y;
            arr_b[index] = texture(textureImage, textureCoord + vec2(i, j) * texelSize).z;
            ++index;
        }
    }

    float color_r = sort_and_get_median(arr_r);
    float color_g = sort_and_get_median(arr_g);
    float color_b = sort_and_get_median(arr_b);
		
    return vec4(color_r, color_g, color_b, 0);
}

void main()
{
    switch (outputMode)
    {
        case 1:
        {
            out_color = grayscale();
            break;
        }

        case 2:
        {
            out_color = blur(3);
            break;
        }
        case 3:
        {
            out_color = filtru_median(2);
            break;
        }
        default:
            out_color = texture(textureImage, textureCoord);
            break;
    }
}
