#ifndef HOHEX_CONSOLE_H
#define HOHEX_CONSOLE_H

#include "editor.h"

#define CONSOLE_COMMAND_HELP "help"
#define CONSOLE_COMMAND_OPEN "open"
#define CONSOLE_COMMAND_SAVE "save"
#define CONSOLE_COMMAND_LOG "log"
#define CONSOLE_COMMAND_NEW_EMPTY_FILE "new"
#define CONSOLE_COMMAND_SEARCH "search"

typedef struct console_command_struct console_command;

enum console_command_type
{
  UNDEFINED,
  HELP,
  OPEN,
  SAVE,
  LOG,
  NEW_EMPTY_FILE,
  SEARCH
};

struct console_command_struct
{
  s32 argc;
  u8** argv;
  enum console_command_type type;
};

s32 console_char_handler(Editor_State* es, s32 key);
void update_console();

internal console_command console_parse_command(u8* command, s32 command_size);
internal void console_free_parsed_command(console_command cs_command);
internal void console_execute_command(console_command cs_command);

internal void run_help_command(s32 argc, u8* argv[]);
internal void run_open_command(s32 argc, u8* argv[]);
internal void run_save_command(s32 argc, u8* argv[]);
internal void run_log_command(s32 argc, u8* argv[]);
internal void run_new_empty_file_command(s32 argc, u8* argv[]);
internal void run_search_command(s32 argc, u8* argv[]);
internal void run_default_command(s32 argc, u8* argv[]);

#endif
