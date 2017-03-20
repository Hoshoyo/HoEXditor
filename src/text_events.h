#ifndef HOHEX_TEXT_EVENTS
#define HOHEX_TEXT_EVENTS

#include "common.h"
#include "text.h"
#include "editor.h"

#define MAX_UNDO_MEM 100
#define MAX_REDO_MEM 100

#define HO_UNDO_STACK 0
#define HO_REDO_STACK 1

#define MAX_KEYS_PER_ACTION_COMMAND 5
#define MAX_ACTION_COMMANDS 100

typedef struct ho_search_result_struct ho_search_result;
typedef struct ho_action_command_struct ho_action_command;
typedef struct ho_text_events_struct ho_text_events;
typedef struct ho_action_item_struct ho_action_item;
typedef struct ho_aiv_undo_redo_struct ho_aiv_undo_redo;
typedef u8 HO_EVENT_STACK;

enum ho_action_command_type
{
  HO_UNDO,
  HO_REDO,
  HO_COPY,
  HO_CUT,
  HO_PASTE,
  HO_SEARCH,
  HO_REPLACE
};

enum ho_action_type
{
  HO_INSERT_TEXT,
  HO_DELETE_TEXT
};

struct ho_search_result_struct
{
  u64 cursor_position;
  ho_search_result* next;
};

struct ho_action_command_struct
{
  enum ho_action_command_type type;
  u32 associated_keys[MAX_KEYS_PER_ACTION_COMMAND];
  u32 num_associated_keys;
};

struct ho_action_item_struct
{
  enum ho_action_type type;
  void* value;
};

// action item value for undo-redo
struct ho_aiv_undo_redo_struct
{
  u8* text;
  u64 text_size;
  u64 cursor_position;
};

struct ho_text_events_struct
{
  // undo / redo
  ho_action_item undo_items[MAX_UNDO_MEM];
  ho_action_item redo_items[MAX_UNDO_MEM];
  u32 max_undo_items;
  u32 max_redo_items;
  u32 num_undo_items;
  u32 num_redo_items;
  u32 undo_stack_begin;
  u32 redo_stack_begin;

  // action commands
  ho_action_command action_commands[MAX_ACTION_COMMANDS];
  u32 num_action_commands;
};

s32 init_text_events(s32 id);
s32 finalize_text_events(s32 id);
// return number of commands called or -1 if error.
s32 save_file(s32 id, u8* filename);

ho_search_result* search_word(s32 id, u64 cursor_begin, u64 cursor_end, u8* pattern, u64 pattern_length);

void keyboard_call_events(s32 id);
void execute_action_command(s32 id, enum ho_action_command_type type);
void update_action_command(s32 id, enum ho_action_command_type type, u32 num_associated_keys, u32* associated_keys);
void remove_action_command(s32 id, enum ho_action_command_type type);
void handle_char_press(Editor_State* es, u8 key);

// add_undo_item and add_redo_item: u8* text must already be allocated.
void add_undo_item(s32 id, enum ho_action_type type, u8* text, u64 text_size, u64 cursor_position);
void add_redo_item(s32 id, enum ho_action_type type, u8* text, u64 text_size, u64 cursor_position);

void clear_events(s32 id);

internal void handle_char_press_block_text(Editor_State* es, u8 key);
internal void handle_char_press_contiguous_text(Editor_State* es, u8 key);
internal void empty_stack(s32 id, HO_EVENT_STACK stack);
internal s32 push_stack_item(s32 id, HO_EVENT_STACK stack, ho_action_item item);
internal ho_action_item pop_stack_item(s32 id, HO_EVENT_STACK stack);
internal void do_undo(s32 id);
internal void do_redo(s32 id);
internal bool is_stack_empty(s32 id, HO_EVENT_STACK stack);
internal ho_action_item copy_action_item(ho_action_item action_item);
internal void free_action_item(ho_action_item action_item);

internal void print_stack(s32 id, HO_EVENT_STACK stack);

#endif
