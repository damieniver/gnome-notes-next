/* gn-note.c
 *
 * Copyright 2018 Mohammed Sadiq <sadiq@sadiqpk.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#define G_LOG_DOMAIN "gn-note"

#include "config.h"

#include "gn-note.h"
#include "gn-note-buffer.h"
#include "gn-trace.h"

/**
 * SECTION: gn-note
 * @title: GnNote
 * @short_description: The base class for Notes
 */

typedef struct
{
  gint dummy;
} GnNotePrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (GnNote, gn_note, GN_TYPE_ITEM)

enum {
  PROP_0,
  PROP_CONTENT,
  PROP_RAW_CONTENT,
  N_PROPS
};

static GParamSpec *properties[N_PROPS];

static void
gn_note_get_property (GObject    *object,
                      guint       prop_id,
                      GValue     *value,
                      GParamSpec *pspec)
{
  GnNote *self = (GnNote *)object;

  switch (prop_id)
    {
    case PROP_CONTENT:
      g_value_set_string (value, gn_note_get_text_content (self));
      break;

    case PROP_RAW_CONTENT:
      g_value_set_string (value, gn_note_get_raw_content (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static GList *
gn_note_real_get_tags (GnNote *self)
{
  g_assert (GN_IS_NOTE (self));

  return NULL;
}
static void
gn_note_real_set_content_to_buffer (GnNote       *self,
                                    GnNoteBuffer *buffer)
{
  const gchar *title;
  g_autofree gchar *raw_content = NULL;
  g_autofree gchar *full_content = NULL;

  g_assert (GN_IS_NOTE (self));

  title = gn_item_get_title (GN_ITEM (self));
  raw_content = gn_note_get_raw_content (self);

  if (raw_content == NULL || raw_content[0] == '\0')
    full_content = g_strdup (title);
  else
    full_content = g_strconcat (title, "\n",
                                raw_content, NULL);

  if (full_content != NULL)
    gtk_text_buffer_set_text (GTK_TEXT_BUFFER (buffer), full_content, -1);
  else
    gtk_text_buffer_set_text (GTK_TEXT_BUFFER (buffer), "", 0);

  gtk_text_buffer_set_modified (GTK_TEXT_BUFFER (buffer), FALSE);
}

static const gchar *
gn_note_real_get_extension (GnNote *self)
{
  return ".txt";
}

static void
gn_note_class_init (GnNoteClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = gn_note_get_property;

  klass->get_tags = gn_note_real_get_tags;
  klass->set_content_to_buffer = gn_note_real_set_content_to_buffer;
  klass->get_extension = gn_note_real_get_extension;

  /**
   * GnNote:content:
   *
   * The plain text content of the note. Shall be used to feed
   * into tracker, or to search within the note.
   */
  properties[PROP_CONTENT] =
    g_param_spec_string ("content",
                         "Content",
                         "The text content of the note",
                         NULL,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * GnNote:raw-content:
   *
   * The raw-content of the note may include XML/markdown content,
   * or whatever is used to keep the note formatted, if any.
   *
   * This is the text that is feed as the content to save. May
   * include title, color, and other details.
   */
  properties[PROP_RAW_CONTENT] =
    g_param_spec_string ("raw-content",
                         "Raw Content",
                         "The raw content of the note",
                         NULL,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
gn_note_init (GnNote *self)
{
}

/**
 * gn_note_get_text_content:
 * @self: a #GnNote
 *
 * Get the plain content of the note, without any
 * formatting tags, if any
 *
 * Returns (transfer full) (nullable): the plain
 * text content of the note
 */
gchar *
gn_note_get_text_content (GnNote *self)
{
  gchar *content;

  GN_ENTRY;

  g_return_val_if_fail (GN_IS_NOTE (self), NULL);

  content = GN_NOTE_GET_CLASS (self)->get_text_content (self);

  GN_RETURN (content);
}

/**
 * gn_note_set_text_content:
 * @self: a #GnNote
 * @content: The text to set as content
 *
 * Set the plain text content of the note. This should
 * not contain the title of the note.
 */
void
gn_note_set_text_content (GnNote      *self,
                          const gchar *content)
{
  GN_ENTRY;

  g_return_if_fail (GN_IS_NOTE (self));

  GN_NOTE_GET_CLASS (self)->set_text_content (self, content);

  GN_EXIT;
}

/**
 * gn_note_get_raw_content:
 * @self: a #GnNote
 *
 * Get the raw content, may contain XML tags, or
 * whatever is used for formatting.
 *
 * Returns (transfer full) (nullable): the raw text
 * content of the note.
 */
gchar *
gn_note_get_raw_content (GnNote *self)
{
  gchar *content;

  GN_ENTRY;

  g_return_val_if_fail (GN_IS_NOTE (self), NULL);

  content = GN_NOTE_GET_CLASS (self)->get_raw_content (self);

  GN_RETURN (content);
}

/**
 * gn_note_get_markup:
 * @self: a #GnNote
 *
 * Get the note content as Pango markup format
 *
 * Returns (transfer full) (nullable): the markup of @self.
 * Free with g_free().
 */
gchar *
gn_note_get_markup (GnNote *self)
{
  gchar *markup;

  GN_ENTRY;

  g_return_val_if_fail (GN_IS_NOTE (self), NULL);

  markup = GN_NOTE_GET_CLASS (self)->get_markup (self);

  GN_RETURN (markup);
}

/**
 * gn_note_get_tags:
 * @self: a #GnNote
 *
 * Get the list of tags (labels) of @self, if any.
 *
 * Returns: (transfer none): A list of #GnTags, or %NULL
 * if not supported/empty.
 */
GList *
gn_note_get_tags (GnNote *self)
{
  GList *tags;

  GN_ENTRY;

  g_return_val_if_fail (GN_IS_NOTE (self), NULL);

  tags = GN_NOTE_GET_CLASS (self)->get_tags (self);

  GN_RETURN (tags);
}

/**
 * gn_note_set_content_from_buffer:
 * @self: a #GnNote
 * @buffer: a #GtkTextBuffer from which text has to be extracted
 *
 * Set the content of the note from buffer
 */
void
gn_note_set_content_from_buffer (GnNote        *self,
                                 GtkTextBuffer *buffer)
{
  GN_ENTRY;

  g_return_if_fail (GN_IS_NOTE (self));

  GN_NOTE_GET_CLASS (self)->set_content_from_buffer (self, buffer);

  GN_EXIT;
}

/**
 * gn_note_set_content_to_buffer:
 * @self: a #GnNote
 *
 * Replace @buffer content with the content
 * of @self
 */
void
gn_note_set_content_to_buffer (GnNote       *self,
                               GnNoteBuffer *buffer)
{
  GN_ENTRY;

  g_return_if_fail (GN_IS_NOTE (self));
  g_return_if_fail (GN_IS_NOTE_BUFFER (buffer));

  GN_NOTE_GET_CLASS (self)->set_content_to_buffer (self, buffer);

  GN_EXIT;
}

/**
 * gn_note_get_extension:
 * @self: a #GnNote
 *
 * Get the commonly used file extension for the given
 * note type.  Say for example, if @self is a #GnPlainNote
 * ".txt" is returned.
 *
 * Returns: (transfer none): The file extension for the
 * given note
 */
const gchar *
gn_note_get_extension (GnNote *self)
{
  const gchar *extension;

  GN_ENTRY;

  g_return_val_if_fail (GN_IS_NOTE (self), ".txt");

  extension = GN_NOTE_GET_CLASS (self)->get_extension (self);

  GN_RETURN (extension);
}
