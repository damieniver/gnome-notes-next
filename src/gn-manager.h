/* gn-manager.h
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

#include <glib-object.h>

#include "gn-item.h"
#include "gn-provider.h"
#include "gn-settings.h"

G_BEGIN_DECLS

#define GN_TYPE_MANAGER (gn_manager_get_type ())

G_DECLARE_FINAL_TYPE (GnManager, gn_manager, GN, MANAGER, GObject)

GnManager  *gn_manager_get_default            (void);
GnSettings *gn_manager_get_settings           (GnManager *self);
GnProvider *gn_manager_get_default_provider   (GnManager *self,
                                               gboolean   show_disconnected);

GListModel *gn_manager_get_notes_store        (GnManager *self);
GListModel *gn_manager_get_trash_notes_store  (GnManager *self);
GListModel *gn_manager_get_search_store       (GnManager *self);

GnItem     *gn_manager_new_note               (GnManager *self);
void        gn_manager_save_item              (GnManager *self,
                                               GnItem    *item);

void        gn_manager_queue_for_delete       (GnManager  *self,
                                               GListModel *store,
                                               GList      *items);
gboolean    gn_manager_dequeue_delete         (GnManager  *self);
void        gn_manager_trash_queue_items      (GnManager  *self);
void        gn_manager_search                 (GnManager  *self,
                                               const gchar **terms);
GList      *gn_manager_get_providers          (GnManager  *self);

G_END_DECLS
