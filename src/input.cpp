void handle_key_down(s32 key, u32 mod) {
	Editor* e = &editor;
	s32 advance = 1;
	if(MOD_LCTRL_DOWN & mod || MOD_RCTRL_DOWN & mod){
		advance = 8;
	}

	switch(key){
		case VK_UP:{
			cursor_up(e, advance);
		}break;
		case VK_DOWN:{
			cursor_down(e, advance);
		}break;
		case VK_RIGHT:{
			cursor_right(e, advance);
		}break;
		case VK_LEFT:{
			cursor_left(e, advance);
		}break;
	}
}

void handle_char_down(s32 key)
{
}

void handle_key_up(s32 key)
{
}

void handle_mouse_move(s32 x, s32 y)
{
}

void handle_lmouse_down(s32 x, s32 y)
{
}

void handle_rmouse_down(s32 x, s32 y)
{

}

void handle_file_drop(s32 x, s32 y, u8* path)
{
}
