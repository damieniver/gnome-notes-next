/* gn-editor.h
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

#include "gn-item.h"

G_BEGIN_DECLS

#define GN_TYPE_EDITOR (gn_editor_get_type ())

G_DECLARE_FINAL_TYPE (GnEditor, gn_editor, GN, EDITOR, GtkGrid)

GtkWidget *gn_editor_new      (void);
void       gn_editor_save_item  (GnEditor *self);
void       gn_editor_set_item (GnEditor   *self,
                               GListModel *model,
                               GnItem     *item);
GnNote    *gn_editor_get_note (GnEditor *self);
GListModel *gn_editor_get_model (GnEditor *self);
GtkWidget  *gn_editor_get_menu  (GnEditor *self);
void        gn_editor_set_detached (GnEditor *self,
                                    gboolean  detached);

G_END_DECLS
