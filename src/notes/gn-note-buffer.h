/* gn-note-buffer.h
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

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GN_TYPE_NOTE_BUFFER (gn_note_buffer_get_type ())

G_DECLARE_FINAL_TYPE (GnNoteBuffer, gn_note_buffer, GN, NOTE_BUFFER, GtkTextBuffer)

GnNoteBuffer *gn_note_buffer_new (void);
void          gn_note_buffer_apply_tag (GnNoteBuffer *self,
                                        const gchar  *tag_name);
void          gn_note_buffer_remove_all_tags  (GnNoteBuffer *self);
const gchar  *gn_note_buffer_get_name_for_tag (GnNoteBuffer *self,
                                               GtkTextTag   *tag);
void          gn_note_buffer_freeze           (GnNoteBuffer *self);
void          gn_note_buffer_thaw             (GnNoteBuffer *self);

G_END_DECLS
