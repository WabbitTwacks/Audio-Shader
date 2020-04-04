#version 330 core
out vec4 fragColor;

uniform float iTime;			//time from the start of the shader program (in seconds)
uniform vec2 iResolution;		//the resolution of the shader (in pixels)
uniform float iAudioLevelAvg;	//the average audio level  from the last 10ms (0.0 - 1.0)
uniform sampler1D iAudioWave; //a 1d texture containing 512 samples of audio waveform

float plot(vec2 uv, float v)
{
	return 	smoothstep(v-0.002, v, uv.y) 
			- smoothstep(v, v+0.002, uv.y);
}

void main()
{ 
	//convert gl_FragCoord to -0.5 to 0.5 range with 0.0 in the middle 
	//of the texture
	vec2 UV = (gl_FragCoord.xy)/iResolution.xy;

	vec3 c = vec3(1.0f, 0.5f, 0.2f); //bright color
	vec3 c2 = vec3(.3, 0.2, 0.1); //dark color
	
	//convert the level to dB scale
	float level = 20. * log(iAudioLevelAvg + 0.001);
	float minLevel = 20. * log(0.001);
	level = (level - minLevel) * 1./(0. - minLevel); 
	
	//mix the bright and dark color depending on the audio level
	c = mix(c2, c, level*level*5.);
	
	int idx = int(UV.x  * 511.);
	float m = plot(UV, texture(iAudioWave, UV.x).r);

	//c *= d;
	c *= m;
	
	//output 
	fragColor = vec4(c, 1.0f);
}
