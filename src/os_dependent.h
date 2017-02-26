HGLOBAL hglb;

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

s32 close_cliboard()
{
#if defined(_WIN64)
  GlobalUnlock(hglb);
  CloseClipboard();
  return 0;
#endif
}
