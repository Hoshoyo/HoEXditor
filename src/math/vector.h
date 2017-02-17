#ifndef HOHEX_VECTOR_H
#define HOHEX_VECTOR_H
#include <math.h>

typedef struct {
	float x;
	float y;
} vec2;

typedef struct {
	float x;
	float y;
	float z;
} vec3;

typedef struct {
	float x;
	float y;
	float z;
	float w;
} vec4;

inline vec2 make_vec2(float x, float y) {
	return (vec2) { x, y };
}
inline vec3 make_vec3(float x, float y, float z) {
	return (vec3) { x, y, z };
}
inline vec4 make_vec4(float x, float y, float z, float w) {
	return (vec4) { x, y, z, w };
}

inline vec2 add_v2(vec2 l, vec2 r) { return (vec2) { l.x + r.x, l.y + r.y }; }
inline vec3 add_v3(vec3 l, vec3 r) { return (vec3) { l.x + r.x, l.y + r.y, l.z + r.z }; }
inline vec4 add_v4(vec4 l, vec4 r) { return (vec4) { l.x + r.x, l.y + r.y, l.z + r.z, l.w + r.w }; }

inline vec2 sub_v2(vec2 l, vec2 r) { return (vec2) { l.x - r.x, l.y - r.y }; }
inline vec3 sub_v3(vec3 l, vec3 r) { return (vec3) { l.x - r.x, l.y - r.y, l.z - r.z }; }
inline vec4 sub_v4(vec4 l, vec4 r) { return (vec4) { l.x - r.x, l.y - r.y, l.z - r.z, l.w - r.w }; }

inline vec2 mul_v2(vec2 l, vec2 r) { return (vec2) { l.x * r.x, l.y * r.y }; }
inline vec3 mul_v3(vec3 l, vec3 r) { return (vec3) { l.x * r.x, l.y * r.y, l.z * r.z }; }
inline vec4 mul_v4(vec4 l, vec4 r) { return (vec4) { l.x * r.x, l.y * r.y, l.z * r.z, l.w * r.w }; }

inline vec2 div_v2(vec2 l, vec2 r) { return (vec2) { l.x / r.x, l.y / r.y }; }
inline vec3 div_v3(vec3 l, vec3 r) { return (vec3) { l.x / r.x, l.y / r.y, l.z / r.z }; }
inline vec4 div_v4(vec4 l, vec4 r) { return (vec4) { l.x / r.x, l.y / r.y, l.z / r.z, l.w / r.w }; }

inline float length_v2(vec2 v) { return sqrtf(v.x * v.x + v.y * v.y); }
inline float length_v3(vec3 v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); }
inline float length_v4(vec4 v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w); }

inline vec2 normalized_v2(vec2 v) { float l = length(v); return (vec2) {v.x / l, v.y / l}; }
inline vec3 normalized_v3(vec3 v) { float l = length(v); return (vec3) {v.x / l, v.y / l, v.z / l}; }
inline vec4 normalized_v4(vec4 v) { float l = length(v); return (vec4) {v.x / l, v.y / l, v.z / l, v.w / l}; }

inline void normalize_v2(vec2* v) { float l = length(v); v->x /= l; v->y /= l; }
inline void normalize_v3(vec3* v) { float l = length(v); v->x /= l; v->y /= l; v->z /= l; }
inline void normalize_v4(vec4* v) { float l = length(v); v->x /= l; v->y /= l; v->z /= l; v->w /= l; }

inline float dot_v2(vec2 l, vec2 r) { return l.x * r.x + l.y * r.y; }
inline float dot_v3(vec3 l, vec3 r) { return l.x * r.x + l.y * r.y + l.z * r.z; }
inline float dot_v4(vec4 l, vec4 r) { return l.x * r.x + l.y * r.y + l.z * r.z + l.w * r.w; }

inline vec3 cross(vec3 l, vec3 r) {
	vec3 res;
	res.x = l.y * r.z - l.z * r.y;
	res.y = l.x * r.z - l.z * r.x;
	res.z = l.x * r.y - l.y * r.x;
	return res;
}

#endif	// HOHEX_VECTOR_H
