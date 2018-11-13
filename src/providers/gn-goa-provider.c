/* gn-goa-provider.c
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

#define G_LOG_DOMAIN "gn-goa-provider"

#include "config.h"

#include "gn-item.h"
#include "gn-plain-note.h"
#include "gn-goa-provider.h"
#include "gn-utils.h"
#include "gn-trace.h"

/**
 * SECTION: gn-goa-provider
 * @title: GnGoaProvider
 * @short_description: GNOME online accounts provider
 * @include: "gn-goa-provider.h"
 */

struct _GnGoaProvider
{
  GnProvider parent_instance;

  gchar *uid;
  gchar *name;
  gchar *icon_name;

  gchar *domain_name;
  gchar *user_name;
  gchar *location_name;

  GoaObject *goa_object;
  GVolume          *volume;
  GMount           *mount;
  GFile *note_dir;

  GListStore *notes_store;
};

G_DEFINE_TYPE (GnGoaProvider, gn_goa_provider, GN_TYPE_PROVIDER)

static void gn_goa_provider_load_items (GnGoaProvider *self,
                                        GTask         *task,
                                        GCancellable  *cancellable);

static void
gn_goa_provider_finalize (GObject *object)
{
  GnGoaProvider *self = (GnGoaProvider *)object;

  GN_ENTRY;

  g_clear_object (&self->goa_object);

  g_clear_pointer (&self->uid, g_free);
  g_clear_pointer (&self->name, g_free);
  g_clear_pointer (&self->icon_name, g_free);
  g_clear_pointer (&self->location_name, g_free);

  G_OBJECT_CLASS (gn_goa_provider_parent_class)->finalize (object);

  GN_EXIT;
}

static gchar *
gn_goa_provider_get_uid (GnProvider *provider)
{
  return g_strdup (GN_GOA_PROVIDER (provider)->uid);
}

static const gchar *
gn_goa_provider_get_name (GnProvider *provider)
{
  GnGoaProvider *self = GN_GOA_PROVIDER (provider);

  return self->name ? self->name : "";
}

static GIcon *
gn_goa_provider_get_icon (GnProvider  *provider,
                          GError     **error)
{
  GnGoaProvider *self = GN_GOA_PROVIDER (provider);

  return g_icon_new_for_string (self->icon_name, error);
}

static const gchar *
gn_goa_provider_get_location_name (GnProvider *provider)
{
  GnGoaProvider *self = GN_GOA_PROVIDER (provider);

  return self->location_name;
}

static GListStore *
gn_goa_provider_get_notes (GnProvider *provider)
{
  return GN_GOA_PROVIDER (provider)->notes_store;
}

static void
gn_goa_provider_check_note_dir (GTask        *task,
                                gpointer      source_object,
                                gpointer      task_data,
                                GCancellable *cancellable)
{
  GnGoaProvider *self = source_object;
  g_autoptr(GFile) root = NULL;
  g_autoptr(GError) error = NULL;

  GN_ENTRY;

  g_assert (G_IS_TASK (task));
  g_assert (GN_IS_GOA_PROVIDER (self));
  g_assert (!cancellable || G_IS_CANCELLABLE (cancellable));

  root = g_mount_get_root (self->mount);
  self->note_dir = g_file_get_child (root, "Notes");

  /* FIXME: g_file_query_exists() doesn't work. */
  g_file_make_directory (self->note_dir, cancellable, &error);

  if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_EXISTS))
    g_clear_error (&error);

  if (error)
    g_task_return_error (task, g_steal_pointer (&error));
  else
    gn_goa_provider_load_items (self, task, cancellable);

  GN_EXIT;
}

static void
gn_goa_provider_volume_mount_cb (GObject      *object,
                                 GAsyncResult *result,
                                 gpointer      user_data)
{
  GTask *task = user_data;
  GnGoaProvider *self;
  g_autoptr(GError) error = NULL;

  g_assert (G_IS_TASK (task));

  if (!g_volume_mount_finish (G_VOLUME (object), result, &error))
    {
      g_task_return_error (task, g_steal_pointer (&error));
      return;
    }

  self = g_task_get_source_object (task);
  self->mount = g_volume_get_mount (self->volume);

  g_task_run_in_thread (task, gn_goa_provider_check_note_dir);
}

/*
 * This is pretty much a copy from gn-local-provider. May be we
 * can merge the code someday.
 */
static void
gn_goa_provider_load_items (GnGoaProvider *self,
                            GTask         *task,
                            GCancellable  *cancellable)
{
  g_autoptr(GFileEnumerator) enumerator = NULL;
  g_autoptr(GError) error = NULL;
  gpointer file_info_ptr;

  GN_ENTRY;

  g_assert (G_IS_TASK (task));
  g_assert (GN_IS_GOA_PROVIDER (self));
  g_assert (!cancellable || G_IS_CANCELLABLE (cancellable));

  enumerator = g_file_enumerate_children (self->note_dir,
                                          G_FILE_ATTRIBUTE_STANDARD_NAME","
                                          G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
                                          G_FILE_QUERY_INFO_NONE,
                                          cancellable,
                                          &error);
  if (error)
    {
      g_task_return_error (task, g_steal_pointer (&error));
      GN_EXIT;
    }

  while ((file_info_ptr = g_file_enumerator_next_file (enumerator, cancellable, NULL)))
    {
      g_autoptr(GFileInfo) file_info = file_info_ptr;
      g_autoptr(GFile) file = NULL;
      g_autofree gchar *contents = NULL;
      g_autofree gchar *file_name = NULL;
      GnPlainNote *note;
      const gchar *name;
      gchar *end;

      name = g_file_info_get_name (file_info);

      if (!g_str_has_suffix (name, ".txt"))
        continue;

      file_name = g_strdup (name);
      end = g_strrstr (file_name, ".");
      *end = '\0';
      file = g_file_get_child (self->note_dir, name);
      g_file_load_contents (file, cancellable, &contents, NULL, NULL, NULL);

      note = gn_plain_note_new_from_data (contents, -1);

      if (note == NULL)
        continue;

      gn_item_set_uid (GN_ITEM (note), file_name);
      g_object_set_data (G_OBJECT (note), "provider", GN_PROVIDER (self));
      g_object_set_data_full (G_OBJECT (note), "file", g_steal_pointer (&file),
                              g_object_unref);
      g_list_store_append (self->notes_store, note);
      /* self->notes = g_list_prepend (self->notes, note); */
    }

  g_task_return_boolean (task, TRUE);

  GN_EXIT;
}

static void
gn_goa_provider_load_items_async (GnProvider          *provider,
                                  GCancellable        *cancellable,
                                  GAsyncReadyCallback  callback,
                                  gpointer             user_data)
{
  GnGoaProvider *self = (GnGoaProvider *)provider;
  g_autoptr(GTask) task = NULL;
  g_autoptr(GVolumeMonitor) monitor = NULL;
  GoaFiles *goa_files;
  const gchar *uri;

  g_assert (GN_IS_GOA_PROVIDER (self));
  g_assert (!cancellable || G_IS_CANCELLABLE (cancellable));

  monitor = g_volume_monitor_get ();
  goa_files = goa_object_peek_files (self->goa_object);
  uri = goa_files_get_uri (goa_files);
  self->volume = g_volume_monitor_get_volume_for_uuid (monitor, uri);

  task = g_task_new (self, cancellable, callback, user_data);
  g_task_set_source_tag (task, gn_goa_provider_load_items_async);
  g_volume_mount (self->volume,
                  G_MOUNT_MOUNT_NONE,
                  NULL,
                  cancellable,
                  gn_goa_provider_volume_mount_cb,
                  g_steal_pointer (&task));
}

static void
gn_goa_provider_save_item (GTask        *task,
                           gpointer      source_object,
                           gpointer      task_data,
                           GCancellable *cancellable)
{
  GnGoaProvider *self = source_object;
  GnItem *item = task_data;
  const gchar *title;
  g_autofree gchar *content = NULL;
  g_autofree gchar *old_basename = NULL;
  g_autofree gchar *new_basename = NULL;
  gchar *full_content;
  g_autoptr(GError) error = NULL;
  GFile *file;

  g_assert (G_IS_TASK (task));
  g_assert (GN_IS_GOA_PROVIDER (self));
  g_assert (GN_IS_ITEM (item));
  g_assert (!cancellable || G_IS_CANCELLABLE (cancellable));

  title = gn_item_get_title (item);
  content = gn_note_get_raw_content (GN_NOTE (item));
  full_content = g_strconcat (title, "\n", content, NULL);
  file = g_object_get_data (G_OBJECT (item), "file");

  if (file != NULL)
    old_basename = g_file_get_basename (file);

  new_basename = g_strconcat (gn_item_get_title (item),
                              gn_note_get_extension (GN_NOTE (item)),
                              NULL);
  if (g_strcmp0 (old_basename, new_basename) != 0)
    {
      file = g_file_set_display_name (file, new_basename, cancellable, &error);

      if (error)
        {
          g_task_return_error (task, g_steal_pointer (&error));
          return;
        }

      g_object_set_data_full (G_OBJECT (item), "file", file, g_object_unref);
    }

  g_file_replace_contents (file, full_content, strlen (full_content),
                           NULL, FALSE, 0, NULL, cancellable, &error);
  if (error)
    g_task_return_error (task, g_steal_pointer (&error));
  else
    g_task_return_boolean (task, TRUE);
}

static void
gn_goa_provider_save_item_async (GnProvider          *provider,
                                 GnItem              *item,
                                 GCancellable        *cancellable,
                                 GAsyncReadyCallback  callback,
                                 gpointer             user_data)
{
  GnGoaProvider *self = (GnGoaProvider *)provider;
  g_autoptr(GTask) task = NULL;
  GFile *file;

  g_assert (GN_IS_GOA_PROVIDER (self));
  g_assert (GN_IS_ITEM (item));
  g_assert (!cancellable || G_IS_CANCELLABLE (cancellable));
  file = g_object_get_data (G_OBJECT (item), "file");

  if (file == NULL)
    {
      g_autofree gchar *file_name = NULL;

      file_name = g_strconcat (gn_item_get_title (item),
                               gn_note_get_extension (GN_NOTE (item)), NULL);
      file = g_file_get_child (self->note_dir, file_name);
      g_object_set_data_full (G_OBJECT (item), "file", g_steal_pointer (&file),
                              g_object_unref);
    }

  task = g_task_new (self, cancellable, callback, user_data);
  g_task_set_source_tag (task, gn_goa_provider_save_item_async);
  g_task_set_task_data (task, g_object_ref (item), g_object_unref);
  g_task_run_in_thread (task, gn_goa_provider_save_item);
}

static void
gn_goa_provider_class_init (GnGoaProviderClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GnProviderClass *provider_class = GN_PROVIDER_CLASS (klass);

  object_class->finalize = gn_goa_provider_finalize;

  provider_class->get_uid = gn_goa_provider_get_uid;
  provider_class->get_name = gn_goa_provider_get_name;
  provider_class->get_icon = gn_goa_provider_get_icon;
  provider_class->get_location_name = gn_goa_provider_get_location_name;
  provider_class->get_notes = gn_goa_provider_get_notes;

  provider_class->load_items_async = gn_goa_provider_load_items_async;
  provider_class->save_item_async = gn_goa_provider_save_item_async;
}

static void
gn_goa_provider_init (GnGoaProvider *self)
{
  self->notes_store = g_list_store_new (GN_TYPE_ITEM);
}

GnProvider *
gn_goa_provider_new (GoaObject *object)
{
  GnGoaProvider *self;
  GoaAccount *account;
  GoaFiles *goa_files;

  g_return_val_if_fail (GOA_IS_OBJECT (object), NULL);

  goa_files = goa_object_peek_files (object);
  g_return_val_if_fail (goa_files != NULL, NULL);

  account = goa_object_peek_account (object);

  self = g_object_new (GN_TYPE_GOA_PROVIDER, NULL);
  self->goa_object = g_object_ref (object);
  self->uid = goa_account_dup_id (account);
  self->name = goa_account_dup_provider_name (account);
  self->icon_name = goa_account_dup_provider_icon (account);
  self->location_name = goa_account_dup_presentation_identity (account);

  return GN_PROVIDER (self);
}
