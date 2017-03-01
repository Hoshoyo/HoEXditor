#ifndef HOHEX_OS_DEPENDENT_H
#define HOHEX_OS_DEPENDENT_H

#include "common.h"

s32 open_clipboard();
s32 get_clipboard_content(u8** content);
s32 set_clipboard_content(u8* content, u64 content_size);
s32 close_clipboard();

s32 write_file();

#endif
