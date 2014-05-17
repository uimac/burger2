#version 140
#ifdef GL_ES
precision mediump float;
#endif
#define M_PI 3.14159265358979323846
out vec4 frag_color;
in vec2 uv;
uniform vec2 resolution;

void hsv_to_rgb(out vec3 rgb, in vec3 hsv)
{
	if (hsv.y == 0.0) { rgb.r = rgb.g = rgb.b = hsv.z; return; }
	int i = int(hsv.x * 6.0);
	float f = hsv.x * 6.0 - float(i);
	float p = hsv.z * (1.0 - hsv.y);
	float q = hsv.z * (1.0 - hsv.y * f);
	float t = hsv.z * (1.0 - hsv.y * (1.0 - f));
	float v = hsv.z;
	if (i == 0) {
		rgb.r = v; rgb.g = t; rgb.b = p;
	} else if (i == 1) {
		rgb.r = q; rgb.g = v; rgb.b = p;
	} else if (i == 2) {
		rgb.r = p; rgb.g = v; rgb.b = t;
	} else if (i == 3) {
		rgb.r = p; rgb.g = q; rgb.b = v;
	} else if (i == 4) {
		rgb.r = t; rgb.g = p; rgb.b = v;
	} else if (i == 5) {
		rgb.r = v; rgb.g = p; rgb.b = q;
	}
}

void main()
{
	float radius = resolution.x / 2.0;
	float x = uv.x * resolution.x - radius;
	float y = (1.0 - uv.y) * resolution.y - radius;
	float radius_squared = radius * radius;
	float distance = x * x + y * y;
	if (distance <= radius_squared)
	{
		vec3 hsv;
		vec3 rgb;
		float angle = 180.0 * (1.0 + atan(y, x) / M_PI) + 90.0;
		if (angle > 360.0)
		{
			 angle -= 360.0;
		}
		hsv.x = angle / 360.0;
		hsv.y = sqrt( distance ) / radius;
		hsv.z = 1.0;
		hsv_to_rgb(rgb, hsv);
		frag_color = vec4(rgb, 1.0);
	}
	else
	{
		frag_color = vec4(0.0, 0.0, 0.0, 0.0);
	}
}
