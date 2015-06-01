#version 130

precision highp float;

uniform mat4  model_to_projection;
uniform mat4  model_to_world;
uniform vec3  C_0;
uniform vec3  A_2;
uniform vec4  color;
uniform sampler2D texture;

// Lighting uniforms
uniform vec4  light_position;
uniform vec4  ambient_color;
uniform vec4  diffuse_color;
uniform vec4  specular_color;
uniform float shininess;

varying   vec4  out_position;
varying   vec4  out_eye_position;
varying   vec3  A_0_inter;
varying   vec3  A_1_inter;
varying   vec3  A_2_inter;
varying   vec3  A_3_inter;
varying   vec2  tex_coord;

void main()
{

	float q_tilda = ((A_1_inter.x * A_1_inter.x) + (A_1_inter.y * A_1_inter.y));

	float alpha = 1.0 - q_tilda;

	if(abs(q_tilda) > 1.0)
		discard;
	else if(abs(q_tilda) <= 1.0)
	{
		vec4 color_set = alpha * texture2D(texture, tex_coord);
		color_set.r = clamp(color_set.r * color.r, 0.0, 1.0);
		color_set.g = clamp(color_set.g * color.g, 0.0, 1.0);
		color_set.b = clamp(color_set.b * color.b, 0.0, 1.0);
		color_set.a = clamp(color_set.a * color.a, 0.0, 1.0);
		gl_FragColor = color_set;
	}
	else
	{
		float lambda = 1.0 / (1.0 + (A_1_inter.z * (1.0 - q_tilda)));
		vec3 normal = normalize(A_2_inter + (lambda * A_3_inter));

		vec3 light_direction = normalize(
			vec3(light_position - out_position)
		);

	 	vec3 light_reflection = normalize(
	 		-reflect(light_direction, normal)
	 	);  

	    // calculate Diffuse Term:
	    vec3 diffuse_color_a = clamp(
	   		vec3(diffuse_color) * 
	   		max(dot(normal, light_direction), 0.0),
	   	0.0, 1.0);

		vec3 eye_to_frag = normalize(vec3(out_position - out_eye_position));
	   
		// calculate Specular Term:
		vec3 specular_color_a = clamp(
	   		vec3(specular_color) * 
	   		pow(max(dot(light_reflection, eye_to_frag),0.0), 0.3 * shininess), 0, 1);

	   // write Total Color:
	   gl_FragColor = vec4(vec3(ambient_color) + diffuse_color_a + specular_color_a, 1.0);

	}

}
