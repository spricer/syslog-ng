/*
 * Copyright (c) 2020 Balabit
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */

#include "template/function.h"
#include "plugin.h"

static void
free_element(LogTemplateEnvEntry *self)
{
  g_free(self->key);
  g_free(self->value);
}

LogTemplateEnvFrame *
log_template_build_frame_impl(const gchar *key, const gchar *value, ...)
{
  GArray *frame = g_array_new(FALSE, FALSE, sizeof(LogTemplateEnvEntry));
  g_array_set_clear_func(frame, (GDestroyNotify)free_element);

  LogTemplateEnvEntry entry;
  entry.key = g_strdup(key);
  entry.value = g_strdup(value);

  g_array_append_val(frame, entry);

  va_list args;
  va_start(args, value);

  while (TRUE)
    {
      const gchar *new_key = va_arg(args, gchar *);
      if (!new_key)
        break;

      const gchar *new_value = va_arg(args, gchar *);
      g_assert(new_value);

      entry.key = g_strdup(new_key);
      entry.value = g_strdup(new_value);
      g_array_append_val(frame, entry);
    }

  va_end(args);

  return frame;
}

static void
free_frame(LogTemplateEnvFrame *frame)
{
  g_array_free(frame, TRUE);
}

static const gchar *
find_in_frame(LogTemplateEnvFrame *frame, const gchar *key)
{
  for (int i = 0; i < frame->len; i++)
    {
      LogTemplateEnvEntry *entry = &g_array_index(frame, LogTemplateEnvEntry, i);
      if (!strcmp(entry->key, key))
        return entry->value;
    }

  return NULL;
}

const gchar *
log_template_find_in_env(GList *env, const gchar *key)
{
  GList *frame = env;
  while (frame)
    {
      const gchar *value = find_in_frame(frame->data, key);
      if (value)
        return value;

      frame = g_list_next(frame);
    }

  return NULL;
}

void
log_template_push_frame(LogMessage *self, LogTemplateEnvFrame *frame)
{
  self->eval_env = g_list_append(self->eval_env, frame);
}

void
log_template_pop_frame(LogMessage *self)
{
  LogTemplateEnvFrame *frame = self->eval_env->data;
  self->eval_env = g_list_delete_link(self->eval_env, self->eval_env);
  free_frame(frame);
}
