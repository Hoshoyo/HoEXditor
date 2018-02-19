#pragma once

u32 shader_load(string vert_src, string frag_src);

void immediate_quad();
void immediate_quad(u32 shader, r32 l, r32 r, r32 t, r32 b);