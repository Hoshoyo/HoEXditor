#include "os_dependent.h"

#if defined(_WIN64)
#include <windows.h>
#include <stdio.h>
HGLOBAL hglb;
#endif

s32 open_clipboard()
{
#if defined(_WIN64)
  if (!OpenClipboard(null))
    return -1;
  return 0;
#endif
}

s32 get_clipboard_content(u8** content)
{
#if defined(_WIN64)
  if (!IsClipboardFormatAvailable(CF_TEXT))
    return -1;

  hglb = GetClipboardData(CF_TEXT);
  if (hglb == null)
    return -1;

  LPTSTR lptstr = GlobalLock(hglb);
  *content = lptstr;
  return 0;
#endif
}

s32 set_clipboard_content(u8* content, u64 content_size)
{
#if defined(_WIN64)
  EmptyClipboard();

  hglb = GlobalAlloc(GMEM_MOVEABLE, content_size + sizeof(char));
  // Lock the handle and copy the text to the buffer.

  LPTSTR lptstrCopy = GlobalLock(hglb);
  copy_string(lptstrCopy, content, content_size);
  lptstrCopy[content_size] = 0;

  SetClipboardData(CF_TEXT, hglb);
  return 0;
#endif
}

s32 close_clipboard()
{
#if defined(_WIN64)
  GlobalUnlock(hglb);
  CloseClipboard();
  return 0;
#endif
}

s32 write_file(u8* filename, u8* text, u64 text_size)
{
#if defined(_WIN64)
  s32 written;
  HANDLE hFile = CreateFile(filename, GENERIC_WRITE, 0, null, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, null);
  WriteFile(hFile, text, text_size, &written, null);
  CloseHandle(hFile);

  return written;
#endif
}
