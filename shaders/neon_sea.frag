#version 330 core
#define PI 3.14159
out vec4 fragColor;

uniform float iTime;			//time from the start of the shader program (in seconds)
uniform vec2 iResolution;		//the resolution of the shader (in pixels)
uniform float iAudioLevelAvg;	//the average audio level  from the last 10ms (0.0 - 1.0)

float expImpulse(float x, float k)
{
	float h = k*x;
	return h*exp(1.0-h);
}

float sinCurve(float x, float k)
{
	float a = PI*(k*x - 1.0);
	return sin(a)/a;
}

float plot(vec2 uv, float x, float width)
{
	return 	smoothstep(x - width, x, uv.y) -
			smoothstep(x, x + width, uv.y);
}

float ease(float x)
{
	return x*x*(2.0-x);
}

vec3 pseudoWave(vec2 UV, float level, float offs, float scale)
{
	//draw a line and animate it with the audio level
	float move = iTime *0.6 + level*2.;
	UV.x += move - 0.5;
	
	float f = sin( iTime + /*+ level*150. */+ UV.x*2.)*1.4*level;
	f += sin(iTime*2. +  level*100. + UV.x*3.)*.2*level;
	f += sin(iTime*4. +  level*60. + UV.x*5.)*.1*level;
	f += sin( iTime*8. + level*70. + UV.x*10.)*.05*level;

	level = pow(level, 0.8) + .1;

	vec3 d = vec3(0.);
	d.r = plot(UV, f +offs, 0.01 * scale) * level*2.;
	d.g = plot(UV, f + offs, 0.05 * scale) * level*2.;
	d.b = plot(UV, f + offs, 0.1 * scale) * level*2.;

	return d;
}

void main()
{ 
	//convert gl_FragCoord to -0.5 to 0.5 range with 0.0 in the middle 
	//of the texture
	vec2 UV = (gl_FragCoord.xy - .5 * iResolution)/iResolution.y;
	//get the square pixel size
	vec2 pixel = vec2(1.0/iResolution.y);

	vec3 c = vec3(1.0, 1.0, 1.0);
	vec3 c2 = vec3(.2, 0.1, 0.5);
	c2.r = smoothstep(-.5, .5, UV.y);
	c2.r = c2.r * c2.r * .9;

	//float level = expImpulse(iAudioLevelAvg, .4);
	float level = iAudioLevelAvg;

	vec3 d = pseudoWave(UV, level, -0.05, 1.);
	d += pseudoWave(UV, level/2., 0.1, .5);
	d += pseudoWave(UV, level/4., 0.15, .25);
	d += pseudoWave(UV, level/8., 0.175, .125);
	d += pseudoWave(UV, level*2., -.3, 2.);

	c = mix(c2, d, 0.7);
	
	//output 
	fragColor = vec4(c, 1.0f);
}
