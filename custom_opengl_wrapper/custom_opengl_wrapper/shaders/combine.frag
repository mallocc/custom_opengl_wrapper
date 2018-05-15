#version 400 core



// ins
in vec3 o_color;
in vec2 o_uv;

// uniforms
uniform mat4 u_m;
uniform mat4 u_v;
uniform mat4 u_p;

uniform sampler2D u_tex0;
uniform sampler2D u_tex1;

out vec4 out_color;

void main() 
{	
	vec3 final_color = texture2D(u_tex0,o_uv).rgb + texture2D(u_tex1,o_uv).rgb;
	final_color /= 2.0f;

// apply fragment color
	out_color = vec4(final_color,1);
}
