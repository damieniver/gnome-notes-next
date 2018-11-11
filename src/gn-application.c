/* gn-application.c
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

#define G_LOG_DOMAIN "gn-application"

#include "config.h"

#include <glib/gi18n.h>

#include "gn-manager.h"
#include "gn-utils.h"
#include "gn-note.h"
#include "gn-window.h"
#include "gn-application.h"
#include "gn-trace.h"

/**
 * SECTION: gn-application
 * @title: gn-application
 * @short_description: Base Application class
 * @include: "gn-application.h"
 */

struct _GnApplication
{
  GtkApplication  parent_instance;

  GnWindow *window;
};

G_DEFINE_TYPE (GnApplication, gn_application, GTK_TYPE_APPLICATION)

static GOptionEntry cmd_options[] = {
  {
    "quit", 'q', G_OPTION_FLAG_NONE,
    G_OPTION_ARG_NONE, NULL,
    N_("Quit all running instances of the application"), NULL
  },
  {
    "version", 0, G_OPTION_FLAG_NONE,
    G_OPTION_ARG_NONE, NULL,
    N_("Show release version"), NULL
  },
  { NULL }
};

static void
gn_application_detach_editor_window (GSimpleAction *action,
                                     GVariant      *parameter,
                                     gpointer       user_data)
{
  GnApplication *self = user_data;
  GnWindow *window;
  GListModel *model;
  GnNote *note;

  g_assert (GN_IS_APPLICATION (self));

  if (!gn_window_steal_note (self->window, &note, &model))
    g_return_if_reached ();

  window = gn_window_new_with_note (self, note, model);

  gtk_window_present (GTK_WINDOW (window));
}

static gint
gn_application_handle_local_options (GApplication *application,
                                     GVariantDict *options)
{
  if (g_variant_dict_contains (options, "version"))
    {
      g_print ("%s %s\n", PACKAGE, PACKAGE_VERSION);
      return 0;
    }

  return -1;
}

static void
gn_application_add_actions (GnApplication *self)
{
  static const GActionEntry application_entries[] = {
    { "detach-editor", gn_application_detach_editor_window },
  };

  g_assert (GN_IS_APPLICATION (self));

  g_action_map_add_action_entries (G_ACTION_MAP (self), application_entries,
                                   G_N_ELEMENTS (application_entries), self);
}

static void
gn_application_startup (GApplication *application)
{
  g_autoptr(GtkCssProvider) css_provider = NULL;

  G_APPLICATION_CLASS (gn_application_parent_class)->startup (application);

  gn_application_add_actions (GN_APPLICATION (application));

  css_provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_resource (css_provider,
                                       "/org/sadiqpk/notes/css/style.css");

  gtk_style_context_add_provider_for_display (gdk_display_get_default (),
                                              GTK_STYLE_PROVIDER (css_provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

static gint
gn_application_command_line (GApplication            *application,
                             GApplicationCommandLine *command_line)
{
  GVariantDict *options;

  options = g_application_command_line_get_options_dict (command_line);

  if (g_variant_dict_contains (options, "quit"))
    {
      g_application_quit (application);
      return 0;
    }

  g_application_activate (application);

  return 0;
}

static void
gn_application_window_destroy_cb (gpointer  user_data,
                                  GObject  *where_the_object_was)
{
  GnApplication *self = user_data;
  GtkWindow *window;

  g_assert (GN_IS_APPLICATION (self));

  window = gtk_application_get_active_window (GTK_APPLICATION (self));

  if (window == NULL)
    return;

  gn_window_set_as_main (GN_WINDOW (window));
  self->window = GN_WINDOW (window);
  g_object_weak_ref (G_OBJECT (self->window),
                     gn_application_window_destroy_cb, self);
}

static void
gn_application_activate (GApplication *application)
{
  GnApplication *self = (GnApplication *)application;

  if (self->window == NULL)
    {
      self->window = GN_WINDOW (gn_window_new (self));
      g_object_weak_ref (G_OBJECT (self->window),
                         gn_application_window_destroy_cb, self);
    }

  gtk_window_present (GTK_WINDOW (self->window));
}

static void
gn_application_shutdown (GApplication *application)
{
  g_object_run_dispose (G_OBJECT (gn_manager_get_default ()));

  G_APPLICATION_CLASS (gn_application_parent_class)->shutdown (application);
}

static void
gn_application_class_init (GnApplicationClass *klass)
{
  GApplicationClass *application_class = G_APPLICATION_CLASS (klass);

  g_assert (GN_IS_MAIN_THREAD ());

  application_class->handle_local_options = gn_application_handle_local_options;
  application_class->startup = gn_application_startup;
  application_class->command_line = gn_application_command_line;
  application_class->activate = gn_application_activate;
  application_class->shutdown = gn_application_shutdown;
}

static void
gn_application_init (GnApplication *self)
{
  g_application_add_main_option_entries (G_APPLICATION (self), cmd_options);

  g_set_application_name (_("GNOME Notes"));
  gtk_window_set_default_icon_name (PACKAGE_ID);
}

GnApplication *
gn_application_new (void)
{
  return g_object_new (GN_TYPE_APPLICATION,
                       "application-id", PACKAGE_ID,
                       "flags", G_APPLICATION_HANDLES_COMMAND_LINE,
                       NULL);
}
