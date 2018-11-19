/* gn-provider.c
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

#define G_LOG_DOMAIN "gn-provider"

#include "config.h"

#include "gn-note.h"
#include "gn-provider.h"
#include "gn-trace.h"

/**
 * SECTION: gn-provider
 * @title: GnProvider
 * @short_description: Base class for providers
 * @include: "gn-provider.h"
 */

typedef struct
{
  gboolean loaded;
} GnProviderPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (GnProvider, gn_provider, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_UID,
  PROP_NAME,
  PROP_ICON,
  PROP_DOMAIN,
  PROP_USER_NAME,
  N_PROPS
};

enum {
  ITEM_ADDED,
  ITEM_DELETED,
  ITEM_TRASHED,
  ITEM_RESTORED,
  N_SIGNALS
};

static GParamSpec *properties[N_PROPS];
static guint signals[N_SIGNALS];

static void
gn_provider_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  GnProvider *self = (GnProvider *)object;

  switch (prop_id)
    {
    case PROP_UID:
      g_value_set_string (value, gn_provider_get_uid (self));
      break;

    case PROP_NAME:
      g_value_set_string (value, gn_provider_get_name (self));
      break;

    case PROP_ICON:
      g_value_set_object (value, gn_provider_get_icon (self, NULL));
      break;

    case PROP_DOMAIN:
      g_value_set_string (value, gn_provider_get_domain (self));
      break;

    case PROP_USER_NAME:
      g_value_set_string (value, gn_provider_get_user_name (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gn_provider_finalize (GObject *object)
{
  GN_ENTRY;

  G_OBJECT_CLASS (gn_provider_parent_class)->finalize (object);

  GN_EXIT;
}

static const gchar *
gn_provider_real_get_name (GnProvider *self)
{
  g_assert (GN_IS_PROVIDER (self));

  /* Derived classes should implement this, if supported */
  return "";
}

static GIcon *
gn_provider_real_get_icon (GnProvider  *self,
                           GError     **error)
{
  g_assert (GN_IS_PROVIDER (self));

  /* Derived classes should implement this, if supported */
  return NULL;
}

static gboolean
gn_provider_real_get_rgba (GnProvider *self,
                           GdkRGBA    *rgba)
{
  g_assert (GN_IS_PROVIDER (self));

  /* Derived classes should implement this, if supported */
  return FALSE;
}

static const gchar *
gn_provider_real_get_location_name (GnProvider *self)
{
  g_assert (GN_IS_PROVIDER (self));

  /* Derived classes should implement this, if supported */
  return "";
}

static GListStore *
gn_provider_real_get_notes (GnProvider *self)
{
  g_assert (GN_IS_PROVIDER (self));

  /* Derived classes should implement this, if supported */
  return NULL;
}

static GListStore *
gn_provider_real_get_tags (GnProvider *self)
{
  g_assert (GN_IS_PROVIDER (self));

  /* Derived classes should implement this, if supported */
  return NULL;
}

static GListStore *
gn_provider_real_get_trash_notes (GnProvider *self)
{
  g_assert (GN_IS_PROVIDER (self));

  /* Derived classes should implement this, if supported */
  return NULL;
}

static GListStore *
gn_provider_real_get_notebooks (GnProvider *self)
{
  g_assert (GN_IS_PROVIDER (self));

  /* Derived classes should implement this, if supported */
  return NULL;
}

static gboolean
gn_provider_real_load_items (GnProvider    *self,
                             GCancellable  *cancellable,
                             GError       **error)
{
  g_set_error_literal (error,
                       G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                       "Loading items synchronously not supported");

  return FALSE;
}

static void
gn_provider_real_load_items_async (GnProvider          *self,
                                   GCancellable        *cancellable,
                                   GAsyncReadyCallback  callback,
                                   gpointer             user_data)
{
  g_task_report_new_error (self, callback, user_data,
                           gn_provider_real_load_items_async,
                           G_IO_ERROR,
                           G_IO_ERROR_NOT_SUPPORTED,
                           "Loading items asynchronously not supported");
}

static gboolean
gn_provider_real_load_items_finish (GnProvider    *self,
                                    GAsyncResult  *result,
                                    GError       **error)
{
  return g_task_propagate_boolean (G_TASK (result), error);
}

static void
gn_provider_real_save_item_async (GnProvider          *self,
                                  GnItem              *item,
                                  GCancellable        *cancellable,
                                  GAsyncReadyCallback  callback,
                                  gpointer             user_data)
{
  g_task_report_new_error (self, callback, user_data,
                           gn_provider_real_save_item_async,
                           G_IO_ERROR,
                           G_IO_ERROR_NOT_SUPPORTED,
                           "Saving item asynchronously not supported");
}

static gboolean
gn_provider_real_save_item_finish (GnProvider    *self,
                                   GAsyncResult  *result,
                                   GError       **error)
{
  return g_task_propagate_boolean (G_TASK (result), error);
}

static gboolean
gn_provider_real_trash_item (GnProvider    *provider,
                             GnItem        *item,
                             GCancellable  *cancellable,
                             GError       **error)
{
  g_set_error_literal (error,
                       G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                       "Trashing item synchronously not supported");

  return FALSE;
}

static void
gn_provider_real_trash_item_async (GnProvider          *self,
                                   GnItem              *item,
                                   GCancellable        *cancellable,
                                   GAsyncReadyCallback  callback,
                                   gpointer             user_data)
{
  g_task_report_new_error (self, callback, user_data,
                           gn_provider_real_trash_item_async,
                           G_IO_ERROR,
                           G_IO_ERROR_NOT_SUPPORTED,
                           "Trashing item asynchronously not supported");
}

static gboolean
gn_provider_real_trash_item_finish (GnProvider    *self,
                                     GAsyncResult  *result,
                                     GError       **error)
{
  return g_task_propagate_boolean (G_TASK (result), error);
}

static void
gn_provider_real_restore_item_async (GnProvider          *self,
                                     GnItem              *item,
                                     GCancellable        *cancellable,
                                     GAsyncReadyCallback  callback,
                                     gpointer             user_data)
{
  g_task_report_new_error (self, callback, user_data,
                           gn_provider_real_restore_item_async,
                           G_IO_ERROR,
                           G_IO_ERROR_NOT_SUPPORTED,
                           "Restoring item asynchronously not supported");
}

static gboolean
gn_provider_real_restore_item_finish (GnProvider    *self,
                                     GAsyncResult  *result,
                                     GError       **error)
{
  return g_task_propagate_boolean (G_TASK (result), error);
}

static void
gn_provider_real_delete_item_async (GnProvider          *self,
                                    GnItem              *item,
                                    GCancellable        *cancellable,
                                    GAsyncReadyCallback  callback,
                                    gpointer             user_data)
{
  g_task_report_new_error (self, callback, user_data,
                           gn_provider_real_delete_item_async,
                           G_IO_ERROR,
                           G_IO_ERROR_NOT_SUPPORTED,
                           "Deleting item asynchronously not supported");
}

static gboolean
gn_provider_real_delete_item_finish (GnProvider    *self,
                                     GAsyncResult  *result,
                                     GError       **error)
{
  return g_task_propagate_boolean (G_TASK (result), error);
}

static void
gn_provider_class_init (GnProviderClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = gn_provider_get_property;
  object_class->finalize = gn_provider_finalize;

  klass->get_name = gn_provider_real_get_name;
  klass->get_icon = gn_provider_real_get_icon;
  klass->get_rgba = gn_provider_real_get_rgba;
  klass->get_location_name = gn_provider_real_get_location_name;
  klass->get_notes = gn_provider_real_get_notes;
  klass->get_tags = gn_provider_real_get_tags;
  klass->get_trash_notes = gn_provider_real_get_trash_notes;
  klass->get_notebooks = gn_provider_real_get_notebooks;

  klass->load_items = gn_provider_real_load_items;
  klass->load_items_async = gn_provider_real_load_items_async;
  klass->load_items_finish = gn_provider_real_load_items_finish;
  klass->save_item_async = gn_provider_real_save_item_async;
  klass->save_item_finish = gn_provider_real_save_item_finish;
  klass->trash_item = gn_provider_real_trash_item;
  klass->trash_item_async = gn_provider_real_trash_item_async;
  klass->trash_item_finish = gn_provider_real_trash_item_finish;
  klass->restore_item_async = gn_provider_real_restore_item_async;
  klass->restore_item_finish = gn_provider_real_restore_item_finish;
  klass->delete_item_async = gn_provider_real_delete_item_async;
  klass->delete_item_finish = gn_provider_real_delete_item_finish;

  properties[PROP_UID] =
    g_param_spec_string ("uid",
                         "Uid",
                         "A unique id for the provider",
                         NULL,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  properties[PROP_NAME] =
    g_param_spec_string ("name",
                         "Name",
                         "The name of the provider",
                         NULL,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  properties[PROP_ICON] =
    g_param_spec_object ("icon",
                         "Icon",
                         "The icon for the provider",
                         G_TYPE_ICON,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  properties[PROP_DOMAIN] =
    g_param_spec_string ("domain",
                         "Domain",
                         "The domain name of the provider",
                         NULL,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  properties[PROP_USER_NAME] =
    g_param_spec_string ("user-name",
                         "User Name",
                         "The user name used for the provider",
                         NULL,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPS, properties);

  /**
   * GnProvider::item-added:
   * @self: a #GnProvider
   * @item: a #GnItem
   *
   * item-added signal is emitted when a new item is added
   * to the provider.
   */
  signals [ITEM_ADDED] =
    g_signal_new ("item-added",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1, GN_TYPE_ITEM);

  /**
   * GnProvider::item-deleted:
   * @self: a #GnProvider
   * @item: a #GnItem
   *
   * item-deleted signal is emitted when a new item is deleted
   * from the provider.
   */
  signals [ITEM_DELETED] =
    g_signal_new ("item-deleted",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1, GN_TYPE_ITEM);

  /**
   * GnProvider::item-trashed:
   * @self: a #GnProvider
   * @item: a #GnItem
   *
   * item-trashed signal is emitted when an item is trashed
   * in the provider.
   */
  signals [ITEM_TRASHED] =
    g_signal_new ("item-trashed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1, GN_TYPE_ITEM);

  /**
   * GnProvider::item-restored:
   * @self: a #GnProvider
   * @item: a #GnItem
   *
   * item-restored signal is emitted when an item is restored
   * from the trash in the provider.
   */
  signals [ITEM_RESTORED] =
    g_signal_new ("item-restored",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1, GN_TYPE_ITEM);
}

static void
gn_provider_init (GnProvider *self)
{
}

/**
 * gn_provider_get_uid:
 * @self: a #GnProvider
 *
 * Get the unique id of the provider
 *
 * Returns: (transfer full) (nullable): the uid
 * of the provider. Free with g_free().
 */
gchar *
gn_provider_get_uid (GnProvider *self)
{
  gchar *uid;

  GN_ENTRY;

  g_return_val_if_fail (GN_IS_PROVIDER (self), NULL);

  uid = GN_PROVIDER_GET_CLASS (self)->get_uid (self);

  GN_RETURN (uid);
}

/**
 * gn_provider_get_name:
 * @self: a #GnProvider
 *
 * Get the name of the provider
 *
 * Returns: (transfer none): the name of the provider.
 */
const gchar *
gn_provider_get_name (GnProvider *self)
{
  const gchar *name;

  GN_ENTRY;

  g_return_val_if_fail (GN_IS_PROVIDER (self), NULL);

  name = GN_PROVIDER_GET_CLASS (self)->get_name (self);

  GN_RETURN (name);
}

/**
 * gn_item_get_icon:
 * @self: a #GnProvider
 *
 * Get the #GIcon of the provider.
 *
 * Returns: (transfer full) (nullable): the icon
 * of the provider. Free with g_object_unref().
 */
GIcon *
gn_provider_get_icon (GnProvider  *self,
                      GError     **error)
{
  GIcon *icon;

  GN_ENTRY;

  g_return_val_if_fail (GN_IS_PROVIDER (self), NULL);

  icon = GN_PROVIDER_GET_CLASS (self)->get_icon (self, error);

  GN_RETURN (icon);
}

/**
 * gn_item_get_rgba:
 * @self: a #GnProvider
 * @rgba: (out): A #GdkRGBA
 *
 * Get the #GdkRGBA color for the @provider.  If @provider
 * has an icon set, the provider may not have any #GdkRGBA
 * set.  See gn_provider_get_icon().
 *
 * Returns: %TRUE if @rgba is set.  %FALSE otherwise.
 */
gboolean
gn_provider_get_rgba (GnProvider *self,
                      GdkRGBA    *rgba)
{
  gboolean ret;

  GN_ENTRY;

  g_return_val_if_fail (GN_IS_PROVIDER (self), FALSE);
  g_return_val_if_fail (rgba != NULL, FALSE);

  ret = GN_PROVIDER_GET_CLASS (self)->get_rgba (self, rgba);

  GN_RETURN (ret);
}

/**
 * gn_provider_get_domain:
 * @self: a #GnProvider
 *
 * Get the domain name of the provider
 *
 * Returns: (transfer full) (nullable): the domain
 * name of the provider. Free with g_free().
 */
gchar *
gn_provider_get_domain (GnProvider *self)
{
  gchar *domain;

  GN_ENTRY;

  g_return_val_if_fail (GN_IS_PROVIDER (self), NULL);

  domain = GN_PROVIDER_GET_CLASS (self)->get_domain (self);

  GN_RETURN (domain);
}

/**
 * gn_provider_get_user_name:
 * @self: a #GnProvider
 *
 * Get the user name used to connect to the provider.
 *
 * Returns: (transfer full) (nullable): the user name
 * connected to the provider. Free with g_free().
 */
gchar *
gn_provider_get_user_name (GnProvider *self)
{
  gchar *user_name;

  GN_ENTRY;

  g_return_val_if_fail (GN_IS_PROVIDER (self), NULL);

  user_name = GN_PROVIDER_GET_CLASS (self)->get_user_name (self);

  GN_RETURN (user_name);
}

/**
 * gn_provider_get_location_name:
 * @self: a #GnProvider
 *
 * Get a human readable location name that represents the provider
 * location.
 *
 * Say for example: A local provider shall return "On this computer"
 *
 * Returns: (transfer none) : A string reperesenting the location
 * of the provider.
 */
const gchar *
gn_provider_get_location_name (GnProvider *self)
{
  const gchar *location_name;

  GN_ENTRY;

  g_return_val_if_fail (GN_IS_PROVIDER (self), "");

  location_name = GN_PROVIDER_GET_CLASS (self)->get_location_name (self);

  GN_RETURN (location_name);
}

/**
 * gn_provider_load_items:
 * @self: a #GnProvider
 * @cancellable: (nullable): a #GCancellable or %NULL
 * @error: A #GError
 *
 * Synchronously load all items (Notes and Notebooks)
 * from the provider.
 *
 * Returns: %TRUE if provider items loaded successfully.
 * %FALSE otherwise.
 */
gboolean
gn_provider_load_items (GnProvider    *self,
                        GCancellable  *cancellable,
                        GError       **error)
{
  GnProviderPrivate *priv = gn_provider_get_instance_private (self);
  gboolean ret;

  GN_ENTRY;

  g_return_val_if_fail (GN_IS_PROVIDER (self), FALSE);
  g_return_val_if_fail (!cancellable || G_IS_CANCELLABLE (cancellable), FALSE);
  g_return_val_if_fail (priv->loaded == FALSE, FALSE);

  ret = GN_PROVIDER_GET_CLASS (self)->load_items (self, cancellable, error);

  if (ret)
    priv->loaded = TRUE;

  GN_RETURN (ret);
}

/**
 * gn_provider_load_items_async:
 * @self: a #GnProvider
 * @cancellable: (nullable): a #GCancellable or %NULL
 * @callback: a #GAsyncReadyCallback, or %NULL
 * @user_data: closure data for @callback
 *
 * Asynchronously load all items (Notes and Notebooks)
 * from the provider.
 *
 * @callback should complete the operation by calling
 * gn_provider_load_items_finish().
 */
void
gn_provider_load_items_async (GnProvider          *self,
                              GCancellable        *cancellable,
                              GAsyncReadyCallback  callback,
                              gpointer             user_data)
{
  GnProviderPrivate *priv = gn_provider_get_instance_private (self);

  GN_ENTRY;

  g_return_if_fail (GN_IS_PROVIDER (self));
  g_return_if_fail (!cancellable || G_IS_CANCELLABLE (cancellable));
  g_return_if_fail (priv->loaded == FALSE);

  GN_PROVIDER_GET_CLASS (self)->load_items_async (self, cancellable,
                                                  callback, user_data);

  GN_EXIT;
}

/**
 * gn_provider_load_items_finish:
 * @self: a #GnProvider
 * @result: a #GAsyncResult provided to callback
 * @error: a location for a #GError or %NULL
 *
 * Completes an asynchronous loading of items initiated
 * with gn_provider_load_items_async().
 *
 * Returns: %TRUE if items loaded successfully. %FALSE otherwise.
 */
gboolean
gn_provider_load_items_finish (GnProvider    *self,
                               GAsyncResult  *result,
                               GError       **error)
{
  GnProviderPrivate *priv = gn_provider_get_instance_private (self);
  gboolean ret;

  GN_ENTRY;

  g_return_val_if_fail (GN_IS_PROVIDER (self), FALSE);
  g_return_val_if_fail (G_IS_ASYNC_RESULT (result), FALSE);

  ret = GN_PROVIDER_GET_CLASS (self)->load_items_finish (self, result, error);

  if (ret)
    priv->loaded = TRUE;

  GN_RETURN (ret);
}

/**
 * gn_provider_save_item_async:
 * @self: a #GnProvider
 * @item: a #GnItem
 * @cancellable: (nullable): a #GCancellable or %NULL
 * @callback: a #GAsyncReadyCallback, or %NULL
 * @user_data: closure data for @callback
 *
 * Asynchronously save the @item. If the item
 * isn't saved at all, a new item (ie, a file, or database
 * entry, or whatever) is created. Else, the old item is
 * updated with the new data.
 *
 * @callback should complete the operation by calling
 * gn_provider_save_item_finish().
 */
void
gn_provider_save_item_async (GnProvider          *self,
                             GnItem              *item,
                             GCancellable        *cancellable,
                             GAsyncReadyCallback  callback,
                             gpointer             user_data)
{
  GN_ENTRY;

  g_return_if_fail (GN_IS_PROVIDER (self));
  g_return_if_fail (GN_IS_ITEM (item));
  g_return_if_fail (!cancellable || G_IS_CANCELLABLE (cancellable));

  g_application_hold (g_application_get_default ());

  GN_PROVIDER_GET_CLASS (self)->save_item_async (self, item,
                                                 cancellable, callback,
                                                 user_data);
  GN_EXIT;
}

/**
 * gn_provider_save_item_finish:
 * @self: a #GnProvider
 * @result: a #GAsyncResult provided to callback
 * @error: a location for a #GError or %NULL
 *
 * Completes saving an item initiated with
 * gn_provider_save_item_async().
 *
 * Returns: %TRUE if the item was saved successfully.
 * %FALSE otherwise.
 */
gboolean
gn_provider_save_item_finish (GnProvider    *self,
                              GAsyncResult  *result,
                              GError       **error)
{
  gboolean ret;

  GN_ENTRY;

  g_return_val_if_fail (GN_IS_PROVIDER (self), FALSE);
  g_return_val_if_fail (G_IS_ASYNC_RESULT (result), FALSE);

  ret = GN_PROVIDER_GET_CLASS (self)->save_item_finish (self, result, error);

  g_application_release (g_application_get_default ());

  GN_RETURN (ret);
}

/**
 * gn_provider_trash_item:
 * @self: a #GnProvider
 * @item: a #GnItem
 * @cancellable: (nullable): a #GCancellable or %NULL
 * @error: a #GError
 *
 * Synchronously trash the @item. If the provider
 * doesn't support trashing, the item will be deleted.
 *
 * Returns: %TRUE if the item was trashed/deleted successfully.
 * %FALSE otherwise.
 */
gboolean
gn_provider_trash_item (GnProvider    *self,
                        GnItem        *item,
                        GCancellable  *cancellable,
                        GError       **error)
{
  gboolean ret;

  GN_ENTRY;

  g_return_val_if_fail (GN_IS_PROVIDER (self), FALSE);
  g_return_val_if_fail (GN_IS_ITEM (item), FALSE);
  g_return_val_if_fail (!cancellable || G_IS_CANCELLABLE (cancellable), FALSE);

  ret = GN_PROVIDER_GET_CLASS (self)->trash_item (self, item,
                                                  cancellable, error);
  GN_RETURN (ret);
}

/* FIXME: Should we delete item if trash not supported? */
/**
 * gn_provider_trash_item_async:
 * @self: a #GnProvider
 * @item: a #GnItem
 * @cancellable: (nullable): a #GCancellable or %NULL
 * @callback: a #GAsyncReadyCallback, or %NULL
 * @user_data: closure data for @callback
 *
 * Asynchronously trash the @item. If the provider
 * doesn't support trashing, the item will be deleted.
 *
 * @callback should complete the operation by calling
 * gn_provider_trash_item_finish().
 */
void
gn_provider_trash_item_async (GnProvider          *self,
                              GnItem              *item,
                              GCancellable        *cancellable,
                              GAsyncReadyCallback  callback,
                              gpointer             user_data)
{
  GN_ENTRY;

  g_return_if_fail (GN_IS_PROVIDER (self));
  g_return_if_fail (GN_IS_ITEM (item));
  g_return_if_fail (!cancellable || G_IS_CANCELLABLE (cancellable));

  GN_PROVIDER_GET_CLASS (self)->trash_item_async (self, item,
                                                  cancellable, callback,
                                                  user_data);
  GN_EXIT;
}

/**
 * gn_provider_trash_item_finish:
 * @self: a #GnProvider
 * @result: a #GAsyncResult provided to callback
 * @error: a location for a #GError or %NULL
 *
 * Completes trashing an item initiated with
 * gn_provider_trash_item_async(). If the item was trashed,
 * "item-trashed" signal will be emitted. And if the item
 * got deleted, "item-deleted" signal will be emitted.
 *
 * Returns: %TRUE if the item was trashed/deleted successfully.
 * %FALSE otherwise.
 */
gboolean
gn_provider_trash_item_finish (GnProvider    *self,
                               GAsyncResult  *result,
                               GError       **error)
{
  gboolean ret;

  GN_ENTRY;

  g_return_val_if_fail (GN_IS_PROVIDER (self), FALSE);
  g_return_val_if_fail (G_IS_ASYNC_RESULT (result), FALSE);

  ret = GN_PROVIDER_GET_CLASS (self)->trash_item_finish (self, result, error);

  GN_RETURN (ret);
}

/**
 * gn_provider_restore_item_async:
 * @self: a #GnProvider
 * @item: a #GnItem
 * @cancellable: (nullable): a #GCancellable or %NULL
 * @callback: a #GAsyncReadyCallback, or %NULL
 * @user_data: closure data for @callback
 *
 * Asynchronously restore the @item.
 *
 * @callback should complete the operation by calling
 * gn_provider_restore_item_finish().
 */
void
gn_provider_restore_item_async (GnProvider          *self,
                                GnItem              *item,
                                GCancellable        *cancellable,
                                GAsyncReadyCallback  callback,
                                gpointer             user_data)
{
  GN_ENTRY;

  g_return_if_fail (GN_IS_PROVIDER (self));
  g_return_if_fail (GN_IS_ITEM (item));
  g_return_if_fail (!cancellable || G_IS_CANCELLABLE (cancellable));

  GN_PROVIDER_GET_CLASS (self)->restore_item_async (self, item,
                                                    cancellable, callback,
                                                    user_data);
  GN_EXIT;
}

/**
 * gn_provider_restore_item_finish:
 * @self: a #GnProvider
 * @result: a #GAsyncResult provided to callback
 * @error: a location for a #GError or %NULL
 *
 * Completes restoring an item initiated with
 * gn_provider_restore_item_async().
 *
 * Returns: %TRUE if the item was restored successfully.
 * %FALSE otherwise.
 */
gboolean
gn_provider_restore_item_finish (GnProvider   *self,
                                 GAsyncResult *result,
                                 GError       **error)
{
  gboolean ret;

  GN_ENTRY;

  g_return_val_if_fail (GN_IS_PROVIDER (self), FALSE);
  g_return_val_if_fail (G_IS_ASYNC_RESULT (result), FALSE);

  ret = GN_PROVIDER_GET_CLASS (self)->restore_item_finish (self, result, error);

  GN_RETURN (ret);
}

/**
 * gn_provider_delete_item_async:
 * @self: a #GnProvider
 * @item: a #GnItem
 * @cancellable: (nullable): a #GCancellable or %NULL
 * @callback: a #GAsyncReadyCallback, or %NULL
 * @user_data: closure data for @callback
 *
 * Asynchronously delete the @item.
 *
 * @callback should complete the operation by calling
 * gn_provider_delete_item_finish().
 */
void
gn_provider_delete_item_async (GnProvider          *self,
                               GnItem              *item,
                               GCancellable        *cancellable,
                               GAsyncReadyCallback  callback,
                               gpointer             user_data)
{
  GN_ENTRY;

  g_return_if_fail (GN_IS_PROVIDER (self));
  g_return_if_fail (GN_IS_ITEM (item));
  g_return_if_fail (!cancellable || G_IS_CANCELLABLE (cancellable));

  GN_PROVIDER_GET_CLASS (self)->delete_item_async (self, item,
                                                   cancellable, callback,
                                                   user_data);
  GN_EXIT;
}

/**
 * gn_provider_delete_item_finish:
 * @self: a #GnProvider
 * @result: a #GAsyncResult provided to callback
 * @error: a location for a #GError or %NULL
 *
 * Completes deleting an item initiated with
 * gn_provider_delete_item_async().
 *
 * Returns: %TRUE if the item was deleted successfully. %FALSE otherwise.
 */
gboolean
gn_provider_delete_item_finish (GnProvider   *self,
                                GAsyncResult *result,
                                GError       **error)
{
  gboolean ret;

  GN_ENTRY;

  g_return_val_if_fail (GN_IS_PROVIDER (self), FALSE);
  g_return_val_if_fail (G_IS_ASYNC_RESULT (result), FALSE);

  ret = GN_PROVIDER_GET_CLASS (self)->delete_item_finish (self, result, error);

  GN_RETURN (ret);
}

/**
 * gn_provider_get_notes:
 * @self: a #GnProvider
 *
 * Get the list of notes loaded by the provider. This
 * should be called only after gn_provider_load_items()
 * or gn_provider_load_items_async() is called.
 *
 * Returns: (transfer none) (nullable): A #GList of
 * #GnItem or %NULL if empty.
 */
GListStore *
gn_provider_get_notes (GnProvider *self)
{
  GListStore *notes;

  GN_ENTRY;

  g_return_val_if_fail (GN_IS_PROVIDER (self), NULL);

  notes = GN_PROVIDER_GET_CLASS (self)->get_notes (self);

  GN_RETURN (notes);
}

/**
 * gn_provider_get_tags:
 * @self: a #GnProvider
 *
 * Get the list of tags/labels loaded by the provider. This
 * should be called only after gn_provider_load_items()
 * or gn_provider_load_items_async() is called.
 *
 * Returns: (transfer none) (nullable): A #GListStore of
 * #GnTag.
 */
GListStore *
gn_provider_get_tags (GnProvider *self)
{
  GListStore *tags;

  GN_ENTRY;

  g_return_val_if_fail (GN_IS_PROVIDER (self), NULL);

  tags = GN_PROVIDER_GET_CLASS (self)->get_tags (self);

  GN_RETURN (tags);
}

/**
 * gn_provider_get_trash_notes:
 * @self: a #GnProvider
 *
 * Get the list of trashed notes loaded by the provider. This
 * should be called only after gn_provider_load_items()
 * or gn_provider_load_items_async() is called.
 *
 * Returns: (transfer none) (nullable): A #GList of
 * #GnItem or %NULL if empty.
 */
GListStore *
gn_provider_get_trash_notes (GnProvider *self)
{
  GListStore *notes;

  GN_ENTRY;

  g_return_val_if_fail (GN_IS_PROVIDER (self), NULL);

  notes = GN_PROVIDER_GET_CLASS (self)->get_trash_notes (self);

  GN_RETURN (notes);
}

/**
 * gn_provider_has_loaded:
 * @self: a #GnProvider
 *
 * Get if the @self has connected and loaded notes.
 *
 * Returns: %TRUE if @self has successfully loaded.
 * %False otherwise.
 */
gboolean
gn_provider_has_loaded (GnProvider *self)
{
  GnProviderPrivate *priv = gn_provider_get_instance_private (self);

  g_return_val_if_fail (GN_IS_PROVIDER (self), FALSE);

  return priv->loaded;
}
