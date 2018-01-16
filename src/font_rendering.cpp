extern Window_State window_state;

internal int font_load(Font_Info* font, const s8* filepath, u32 pixel_point, u32 load_limit)
{
	assert(load_limit <= MAX_UNICODE);

	FT_Library library;
	
	int error = FT_Init_FreeType(&library);
	if (error) {
		printf("Error initializing freetype\n");
		return -1;
	}
	error = FT_New_Face(library, filepath, 0, &font->face);
	if (error == FT_Err_Unknown_File_Format) {
		printf("Error loading font file format\n");
		return -1;
	}
	else if (error) {
		printf("Error loading font\n");
		return -1;
	}

	if (FT_HAS_VERTICAL(font->face)) {
		printf("Font %s has vertical metrics\n", filepath);
		//assert(0);	// @TODO
	}
	if (FT_HAS_KERNING(font->face)) {
		font->kerning = false;
		printf("Font %s has kerning metrics\n", filepath);
		// @TODO
	}

	error = FT_Set_Pixel_Sizes(font->face, 0, pixel_point);

	s32 x_adv = 0, y_adv = 0;
	s32 previous_max_height = 0;

	s32 max_height = (font->face->size->metrics.ascender - font->face->size->metrics.descender) >> 6;
	s32 max_width = font->face->size->metrics.max_advance >> 6;
	s32 num_glyphs = font->face->num_glyphs;
	s32 num_glyphs_loaded = 0;
	s32 size = 2048;
	r32 atlasf_size = (r32)size;
	font->atlas_size = size;

	font->font_size = pixel_point;
	font->max_height = max_height;
	font->max_width = max_width;

	// allocate memory for the texture atlas of the font
	font->atlas_data = (u8*)calloc(1, size * size * 4);

	int max_height_computed = 0;
	int max_height_index = 0;

	for (u32 i = 0; i < load_limit; ++i) {
		u32 index = FT_Get_Char_Index(font->face, i);
		error = FT_Load_Glyph(font->face, index, FT_LOAD_RENDER);

		if (index == 0) {
			continue;
		}

		s32 width = font->face->glyph->bitmap.width;
		s32 height = font->face->glyph->bitmap.rows;

		hm::vec2 topl, topr, botl, botr;

		if (width && height) {
			// if got to the end of the first row, reset x_advance and sum y
			if (x_adv + width >= size) {
				y_adv += previous_max_height;
				x_adv = 0;
			}
			// copy from the FT bitmap to atlas in the correct position
			for (u32 h = 0; h < height; ++h) {
				u8* b = font->face->glyph->bitmap.buffer;
				memory_copy(font->atlas_data + (size * (h + y_adv)) + x_adv, b + width * h, width);
			}
			font->characters[i].botl = { x_adv / atlasf_size,			  y_adv / atlasf_size };
			font->characters[i].botr = { (x_adv + width) / atlasf_size, y_adv / atlasf_size };
			font->characters[i].topl = { x_adv / atlasf_size,			  (y_adv + height) / atlasf_size };
			font->characters[i].topr = { (x_adv + width) / atlasf_size, (y_adv + height) / atlasf_size };

			// keep the packing by getting the max height of the previous row of packing
			if (height > previous_max_height) previous_max_height = height;
			if (height > max_height_computed) {
				max_height_computed = height;
				max_height_index = i;
			}
			x_adv += width;

			// this glyph exists
			font->characters[i].renderable = true;
			font->characters[i].advance = font->face->glyph->advance.x;
			font->characters[i].size[0] = font->face->glyph->bitmap.width;
			font->characters[i].size[1] = font->face->glyph->bitmap.rows;
			font->characters[i].bearing[0] = font->face->glyph->bitmap_left;
			font->characters[i].bearing[1] = font->face->glyph->bitmap_top;

			num_glyphs_loaded += 1;
		}
	}

	FT_Done_Face(font->face);
	FT_Done_FreeType(library);

	font->finish_load = true;
	return 0;
}


void font_finish_load(Font_Info* font)
{
	if (font->finish_load) {
		font->finish_load = false;
		u32 size = font->atlas_size;
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glGenTextures(1, &font->atlas_full_id);
		glBindTexture(GL_TEXTURE_2D, font->atlas_full_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, size, size, 0, GL_RED, GL_UNSIGNED_BYTE, font->atlas_data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

		memory_free(font->atlas_data);
		font->loaded = true;
	}
}

internal void text_buffer_init(Font_Info* font, s64 size) {
	glGenVertexArrays(1, &font->vao);
	glBindVertexArray(font->vao);
	glGenBuffers(1, &font->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, font->vbo);
	glGenBuffers(1, &font->ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, font->ebo);

	const int num_vertices = 4;
	const int num_attribs = 9;

	u16* indices = (u16*)memory_alloc(size * 6 * sizeof(u16));
	float* data = (float*)memory_alloc(size * num_attribs * num_vertices * sizeof(float));

	for (u16 i = 0, j = 0, k = 0; i < size; ++i, j += 4, k += 6) {
		indices[k] = j;
		indices[k + 1] = j + 1;
		indices[k + 2] = j + 3;
		indices[k + 3] = j + 3;
		indices[k + 4] = j + 1;
		indices[k + 5] = j + 2;
	}
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * 6 * sizeof(u16), indices, GL_DYNAMIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, size * num_attribs * num_vertices * sizeof(float), data, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * num_attribs, (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * num_attribs, (void*)(sizeof(float) * 3));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(float) * num_attribs, (void*)(sizeof(float) * (3 + 2)));

	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	font->text_buffer_max_length = size;
	font->text_buffer_length = 0;

	memory_free(indices);
	memory_free(data);
}

internal void text_buffer_realloc(Font_Info* font, s64 new_size) {
	glBindVertexArray(font->vao);
	glDeleteBuffers(1, &font->vbo);
	glDeleteBuffers(1, &font->ebo);

	text_buffer_init(font, new_size);
}

internal void text_buffer_change(float* dest, float* data, s64 size)
{
	const int num_vertices = 4;
	const int num_attribs = 9;
	memcpy(dest, data, size * sizeof(float) * num_attribs * num_vertices);
}

internal void text_buffer_draw(Font_Info* font, s32 size, hm::vec2 window_size)
{
	if (!font->loaded) {
		font_finish_load(font);
		return;
	}

	glBindVertexArray(font->vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, font->ebo);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	RECT rect;
	GetClientRect(win_state.window_handle, &rect);
	s32 width = rect.right - rect.left;
	s32 height = rect.bottom - rect.top;

	hm::mat4 projection = hm::mat4::ortho(0, width, 0, height);
	GLuint loc = glGetUniformLocation(font->shader, "projection");
	glUniformMatrix4fv(loc, 1, GL_TRUE, projection.data);

	glActiveTexture(GL_TEXTURE0);
	if (font->flags & FONT_ASCII_ONLY) {
		glBindTexture(GL_TEXTURE_2D, font->atlas_asci_id);
	} else {
		glBindTexture(GL_TEXTURE_2D, font->atlas_full_id);
	}

	glDrawElements(GL_TRIANGLES, 6 * size, GL_UNSIGNED_SHORT, 0);

	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

}

internal u32 utf8_to_unicode(u8* text, u32* advance) {
	u32 result = 0;
	if (text[0] & 128) {
		// 1xxx xxxx
		if (text[0] > 0xF0) {
			// 4 bytes
			*advance = 4;
			result = ((text[0] & 0x07) << 26) | ((text[1] & 0x3F) << 12) | ((text[2] & 0x3F) << 6) | (text[3] | 0x3F);
		}
		else if (text[0] > 0xE0) {
			// 3 bytes
			*advance = 3;
			result = ((text[0] & 0x0F) << 12) | ((text[1] & 0x3F) << 6) | (text[2] & 0x3F);
		}
		else if (text[0] > 0xC0) {
			// 2 bytes
			*advance = 2;
			result = ((text[0] & 0x1F) << 6) | (text[1] & 0x3F);
		}
		else {
			// continuation byte
			*advance = 1;
			result = text[0];
		}
	} else {
		*advance = 1;
		result = (u32)text[0];
	}
	return result;
}

internal void text_draw(Font_Info* font, s64 offset_, u8* text, s32 length, hm::vec2& position, hm::vec4 color)
{
	Character* characters = font->characters;
	glBindBuffer(GL_ARRAY_BUFFER, font->vbo);
	void* buffer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	u32 offset = 0, num_chars = 0;
	u32 previous_index = 0;
	for (u32 i = 0, c = 0; text[c] != 0; ++i) {
		u32 advance = 0;
		u32 index = utf8_to_unicode(&text[c], &advance);
		c += advance;

		if (!characters[index].renderable)
			index = '.';

		if (font->kerning) {
			FT_Vector delta;
			FT_Get_Kerning(font->face, previous_index, index, FT_KERNING_DEFAULT, &delta);
			position.x += delta.x >> 6;
		}
		previous_index = index;

		GLfloat xpos = (GLfloat)position.x + characters[index].bearing[0];
		GLfloat ypos = (GLfloat)position.y - (characters[index].size[1] - characters[index].bearing[1]);

		GLfloat w = characters[index].size[0];
		GLfloat h = characters[index].size[1];

		float data[] = {
			xpos + w, ypos      , 0.0f, characters[index].topr.x, characters[index].topr.y, color.r, color.g, color.b, color.a,
			xpos, ypos          , 0.0f, characters[index].topl.x, characters[index].topl.y, color.r, color.g, color.b, color.a,
			xpos, ypos + h      , 0.0f, characters[index].botl.x, characters[index].botl.y, color.r, color.g, color.b, color.a,
			xpos + w, ypos + h  , 0.0f, characters[index].botr.x, characters[index].botr.y, color.r, color.g, color.b, color.a
		};

		text_buffer_change((float*)(((u8*)buffer + (sizeof(data) * offset_)) + offset), data, 1);

		offset += sizeof(data);
		num_chars += 1;
		position.x += characters[index].advance >> 6;
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
}

internal int render_text(Font_Info* font, string text, hm::vec2& position, hm::vec4 color) {
	if (font->loaded) {
		if (font->text_buffer_offset + text.length >= font->text_buffer_max_length) {
			text_buffer_realloc(font, font->text_buffer_offset + text.length + 512);
		}
		text_draw(font, font->text_buffer_offset, (u8*)text.data, text.length, position, color);
		font->text_buffer_offset += text.length;
		font->text_buffer_length += text.length;
	} else {
		return 1;
	}
	return 0;
}

internal int render_text_get_info(Font_Info* font, string text_in, hm::vec2& position_out) {
	r32 max_height = position_out.y;
	u8* text = (u8*)text_in.data;

	if (font->loaded) {
		Character* characters = font->characters;

		u32 num_chars = 0;
		u32 previous_index = 0;
		for (u32 i = 0, c = 0; text[c] != 0; ++i) {
			u32 advance = 0;
			u32 index = utf8_to_unicode(&text[c], &advance);
			c += advance;

			if (!characters[index].renderable)
				index = '.';

			if (font->kerning) {
				FT_Vector delta;
				FT_Get_Kerning(font->face, previous_index, index, FT_KERNING_DEFAULT, &delta);
				position_out.x += delta.x >> 6;
			}
			previous_index = index;

			GLfloat xpos = (GLfloat)position_out.x + characters[index].bearing[0];
			GLfloat ypos = (GLfloat)position_out.y - (characters[index].size[1] - characters[index].bearing[1]);

			GLfloat w = characters[index].size[0];
			GLfloat h = characters[index].size[1];

			if (h > max_height) {
				max_height = h;
			}

			num_chars += 1;
			position_out.x += characters[index].advance >> 6;
		}
	} else {
		return 1;
	}
	position_out.y = max_height;
	return 0;
}


internal void font_rendering_flush(Font_Info* font, u32 shader) {
	glUseProgram(shader);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	text_buffer_draw(font, font->text_buffer_length, hm::vec2(font->window_width, font->window_height));
	font->text_buffer_length = 0;
	font->text_buffer_offset = 0;

	glDisable(GL_BLEND);
}