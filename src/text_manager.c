#include "text_manager.h"
#include "memory.h"

internal u8* _tm_buffer;
internal u64 _tm_buffer_size;
internal u64 _tm_cursor_begin;
u64 _tm_text_size;

u32 init_text_api()
{
  init_text();

  _tm_buffer = null;
  _tm_cursor_begin = 0;
  _tm_buffer_size = 0;
  _tm_text_size = 0;

  // dummy
  ho_block* last_block = &(get_first_block_container()->blocks[0]);
  insert_text_in_block(last_block, "Matadores de Imortais, um nome um tanto quanto contraditorio", 0,
    hstrlen("Matadores de Imortais, um nome um tanto quanto contraditorio"), true);
  last_block = append_block(*last_block);
  insert_text_in_block(last_block, ", pois como seria possivel que um ", 0, hstrlen(", pois como seria possivel que um "), true);
  last_block = append_block(*last_block);
  insert_text_in_block(last_block, "ser que e imortal pudesse morrer? Porem foi esse o fruto", 0,
    hstrlen("ser que e imortal pudesse morrer? Porem foi esse o fruto"), true);
  last_block = append_block(*last_block);
  insert_text_in_block(last_block, " da imaginacao de Jairon (tambem conhecido como Hel)", 0,
    hstrlen(" da imaginacao de Jairon (tambem conhecido como Hel)"), true);
  last_block = append_block(*last_block);
  insert_text_in_block(last_block, " que a fundou no ano de 2007 no servidor Ayumel (Ragmaniacos).", 0,
    hstrlen(" que a fundou no ano de 2007 no servidor Ayumel (Ragmaniacos)."), true);
  last_block = append_block(*last_block);
  insert_text_in_block(last_block, "No inicio nao haviam muitos objetivos em relacao a MDI,", 0,
    hstrlen("No inicio nao haviam muitos objetivos em relacao a MDI,"), true);
  last_block = append_block(*last_block);
  insert_text_in_block(last_block, " pois os membros mal sabiam jogar.", 0, hstrlen(" pois os membros mal sabiam jogar."), true);

  _tm_text_size = 353;  // temporary

  return 0;
}

u32 end_text_api()
{
  hfree(_tm_buffer);
  return destroy_text();
}

u8* get_text_buffer(u64 size, u64 cursor_begin)
{
  u32 ret;

  if (_tm_buffer == null)
  {
    _tm_buffer = halloc(sizeof(u8) * size);
  }
  else if (_tm_buffer_size != size)
  {
    hfree(_tm_buffer);
    _tm_buffer = halloc(sizeof(u8) * size);
  }

  _tm_buffer_size = size;
  _tm_cursor_begin = cursor_begin;

  ret = fill_buffer();

  if (!ret)
    return _tm_buffer;
  else
  {
    hfree(_tm_buffer);
    return null;
  }
}

u32 set_cursor_begin(u64 cursor_begin)
{
  _tm_cursor_begin = cursor_begin;
  return fill_buffer();
}

u32 insert_text(u8* text, u64 size, u64 cursor_begin)
{
  return -1;
}

u32 delete_text(u64 cursor_begin, u64 size)
{
  return -1;
}

u32 move_block_data(ho_block* block, u32 initial_block_position, u64 size, u8* memory_position)
{
  u32 i, current_initial_block_position = initial_block_position;
  u64 remaining_to_move = size;
  u8* current_mem_position = memory_position;
  ho_block_container* current_block_container = block->container;
  ho_block* current_block;

  i = block->position_in_container;

  while (current_block_container != null)
  {
    for (; i<current_block_container->num_blocks_in_container; ++i)
    {
      current_block = &current_block_container->blocks[i];

      if (remaining_to_move > (current_block->occupied - current_initial_block_position))
      {
        u32 move_size = current_block->occupied - current_initial_block_position;
        copy_string(current_mem_position, current_block->block_data.data + current_initial_block_position, move_size);
        current_mem_position += move_size;
        remaining_to_move -= move_size;
        current_initial_block_position = 0;
      }
      else
      {
        copy_string(current_mem_position, current_block->block_data.data + current_initial_block_position, remaining_to_move);
        return 0;
      }
    }

    i = 0;
    current_block_container = current_block_container->next;
  }

  error_fatal("move_block_data error: block overflow.", 0);
  return -1;
}

u32 fill_buffer()
{
  u32 block_position;
  ho_block* block = get_initial_block_at_cursor_begin(&block_position);

  if (block != null)
  {
    return move_block_data(block, block_position, _tm_buffer_size, _tm_buffer);
  }
  else
  {
    return -1;
  }
}

ho_block* get_initial_block_at_cursor_begin(u32* block_position)
{
  u64 cursor_position = 0, i;
  ho_block_container* current_block_container = get_first_block_container();
  ho_block* block_aux;
  ho_block* last_block;

  if (current_block_container == null)
  {
    error_fatal("get_initial_block_at_cursor_begin: first block container is nil.\n", 0);
    return null;
  }
  else if (current_block_container->num_blocks_in_container <= 0)
  {
    error_fatal("get_initial_block_at_cursor_begin: no blocks.\n");
    return null;
  }

  while (current_block_container != null)
  {
    for (i=0; i<current_block_container->num_blocks_in_container; ++i)
    {
      last_block = &current_block_container->blocks[i];
      u32 b_occupied = current_block_container->blocks[i].occupied;
      if (cursor_position + b_occupied > _tm_cursor_begin)
      {
        *block_position = _tm_cursor_begin - cursor_position;
        return last_block;
      }

      cursor_position += current_block_container->blocks[i].occupied;
    }

    current_block_container = current_block_container->next;
  }

  *block_position = _tm_cursor_begin - cursor_position;
  return last_block;
}
