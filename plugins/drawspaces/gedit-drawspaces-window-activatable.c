/*
 * Copyright (C) 2008-2014 Ignacio Casal Quinteiro <nacho.resa@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gedit-drawspaces-window-activatable.h"
#include "gedit-drawspaces-app-activatable.h"

#include <glib/gi18n-lib.h>
#include <gedit/gedit-debug.h>
#include <gedit/gedit-view.h>
#include <gedit/gedit-tab.h>
#include <gedit/gedit-window.h>
#include <gedit/gedit-window-activatable.h>
#include <gedit/gedit-utils.h>
#include <libpeas-gtk/peas-gtk-configurable.h>

#define DRAWSPACES_SETTINGS_BASE   "org.gnome.gedit.window_activatables.drawspaces"
#define SETTINGS_KEY_SHOW_WHITE_SPACE "show-white-space"
#define SETTINGS_KEY_DRAW_SPACES   "draw-spaces"

#define GEDIT_DRAWSPACES_WINDOW_ACTIVATABLE_GET_PRIVATE(object) \
				(G_TYPE_INSTANCE_GET_PRIVATE ((object),	\
				GEDIT_TYPE_DRAWSPACES_WINDOW_ACTIVATABLE,		\
				GeditDrawspacesWindowActivatablePrivate))

static void gedit_window_activatable_iface_init (GeditWindowActivatableInterface *iface);
static void peas_gtk_configurable_iface_init (PeasGtkConfigurableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (GeditDrawspacesWindowActivatable,
				gedit_drawspaces_window_activatable,
				PEAS_TYPE_EXTENSION_BASE,
				0,
				G_IMPLEMENT_INTERFACE_DYNAMIC (GEDIT_TYPE_WINDOW_ACTIVATABLE,
							       gedit_window_activatable_iface_init)
				G_IMPLEMENT_INTERFACE_DYNAMIC (PEAS_GTK_TYPE_CONFIGURABLE,
							       peas_gtk_configurable_iface_init))

struct _GeditDrawspacesWindowActivatablePrivate
{
	GSettings *settings;
	GeditWindow *window;
	GtkSourceDrawSpacesFlags flags;

	guint enable : 1;
};

typedef struct _DrawspacesConfigureWidget DrawspacesConfigureWidget;

struct _DrawspacesConfigureWidget
{
	GSettings *settings;
	GtkSourceDrawSpacesFlags flags;

	GtkWidget *content;

	GtkWidget *draw_tabs;
	GtkWidget *draw_spaces;
	GtkWidget *draw_newline;
	GtkWidget *draw_nbsp;
	GtkWidget *draw_leading;
	GtkWidget *draw_text;
	GtkWidget *draw_trailing;
};

enum
{
	COLUMN_LABEL,
	COLUMN_LOCATION
};

enum
{
	PROP_0,
	PROP_APP,
	PROP_WINDOW
};

static void draw_spaces (GeditDrawspacesWindowActivatable *window_activatable);

static void
on_settings_changed (GSettings                        *settings,
		     const gchar                      *key,
		     GeditDrawspacesWindowActivatable *window_activatable)
{
	window_activatable->priv->flags = g_settings_get_flags (window_activatable->priv->settings,
	                                                        SETTINGS_KEY_DRAW_SPACES);

	draw_spaces (window_activatable);
}

static void
on_show_white_space_changed (GSettings                        *settings,
		             const gchar                      *key,
		             GeditDrawspacesWindowActivatable *window_activatable)
{
	window_activatable->priv->enable = g_settings_get_boolean (settings, key);

	draw_spaces (window_activatable);
}

static void
gedit_drawspaces_window_activatable_init (GeditDrawspacesWindowActivatable *window_activatable)
{
	gedit_debug_message (DEBUG_PLUGINS, "GeditDrawspacesWindowActivatable initializing");

	window_activatable->priv = GEDIT_DRAWSPACES_WINDOW_ACTIVATABLE_GET_PRIVATE (window_activatable);

	window_activatable->priv->settings = g_settings_new (DRAWSPACES_SETTINGS_BASE);

	g_signal_connect (window_activatable->priv->settings,
	                  "changed::show-white-space",
	                  G_CALLBACK (on_show_white_space_changed),
	                  window_activatable);
	g_signal_connect (window_activatable->priv->settings,
			  "changed::draw-spaces",
			  G_CALLBACK (on_settings_changed),
			  window_activatable);
}

static void
gedit_drawspaces_window_activatable_dispose (GObject *object)
{
	GeditDrawspacesWindowActivatable *window_activatable = GEDIT_DRAWSPACES_WINDOW_ACTIVATABLE (object);
	GeditDrawspacesWindowActivatablePrivate *priv = window_activatable->priv;

	gedit_debug_message (DEBUG_PLUGINS, "GeditDrawspacesWindowActivatable disposing");

	g_clear_object (&priv->settings);
	g_clear_object (&priv->window);

	G_OBJECT_CLASS (gedit_drawspaces_window_activatable_parent_class)->dispose (object);
}

static void
gedit_drawspaces_window_activatable_set_property (GObject      *object,
                                                  guint         prop_id,
                                                  const GValue *value,
                                                  GParamSpec   *pspec)
{
	GeditDrawspacesWindowActivatable *window_activatable = GEDIT_DRAWSPACES_WINDOW_ACTIVATABLE (object);

	switch (prop_id)
	{
		case PROP_WINDOW:
			window_activatable->priv->window = GEDIT_WINDOW (g_value_dup_object (value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gedit_drawspaces_window_activatable_get_property (GObject    *object,
                                                  guint       prop_id,
                                                  GValue     *value,
                                                  GParamSpec *pspec)
{
	GeditDrawspacesWindowActivatable *window_activatable = GEDIT_DRAWSPACES_WINDOW_ACTIVATABLE (object);

	switch (prop_id)
	{
		case PROP_WINDOW:
			g_value_set_object (value, window_activatable->priv->window);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
draw_spaces (GeditDrawspacesWindowActivatable *window_activatable)
{
	GeditDrawspacesWindowActivatablePrivate *priv;
	GList *views, *l;

	priv = window_activatable->priv;

	views = gedit_window_get_views (priv->window);
	for (l = views; l != NULL; l = g_list_next (l))
	{
		gtk_source_view_set_draw_spaces (GTK_SOURCE_VIEW (l->data),
						 priv->enable ? priv->flags : 0);
	}

	g_list_free (views);
}

static void
tab_added_cb (GeditWindow *window,
	      GeditTab *tab,
	      GeditDrawspacesWindowActivatable *window_activatable)
{
	GeditView *view;

	if (window_activatable->priv->enable)
	{
		view = gedit_tab_get_view (tab);

		gtk_source_view_set_draw_spaces (GTK_SOURCE_VIEW (view),
						 window_activatable->priv->flags);
	}
}

static void
get_config_options (GeditDrawspacesWindowActivatable *window_activatable)
{
	GeditDrawspacesWindowActivatablePrivate *priv = window_activatable->priv;

	priv->enable = g_settings_get_boolean (priv->settings,
					       SETTINGS_KEY_SHOW_WHITE_SPACE);

	priv->flags = g_settings_get_flags (priv->settings,
					    SETTINGS_KEY_DRAW_SPACES);
}

static void
gedit_drawspaces_window_activatable_window_activate (GeditWindowActivatable *activatable)
{
	GeditDrawspacesWindowActivatablePrivate *priv;
	GMenuItem *item;
	GAction *action;

	gedit_debug (DEBUG_PLUGINS);

	priv = GEDIT_DRAWSPACES_WINDOW_ACTIVATABLE (activatable)->priv;

	get_config_options (GEDIT_DRAWSPACES_WINDOW_ACTIVATABLE (activatable));

	action = g_settings_create_action (priv->settings,
	                                   SETTINGS_KEY_SHOW_WHITE_SPACE);
	g_action_map_add_action (G_ACTION_MAP (priv->window),
	                         action);
	g_object_unref (action);

	if (priv->enable)
	{
		draw_spaces (GEDIT_DRAWSPACES_WINDOW_ACTIVATABLE (activatable));
	}

	g_signal_connect (priv->window, "tab-added",
			  G_CALLBACK (tab_added_cb), activatable);
}

static void
gedit_drawspaces_window_activatable_window_deactivate (GeditWindowActivatable *activatable)
{
	GeditDrawspacesWindowActivatablePrivate *priv;
	GtkUIManager *manager;

	gedit_debug (DEBUG_PLUGINS);

	priv = GEDIT_DRAWSPACES_WINDOW_ACTIVATABLE (activatable)->priv;

	g_action_map_remove_action (G_ACTION_MAP (priv->window),
	                            SETTINGS_KEY_SHOW_WHITE_SPACE);

	priv->enable = FALSE;
	draw_spaces (GEDIT_DRAWSPACES_WINDOW_ACTIVATABLE (activatable));

	g_signal_handlers_disconnect_by_func (priv->window, tab_added_cb,
					      activatable);
}

static void
widget_destroyed (GtkWidget *obj, gpointer widget_pointer)
{
	DrawspacesConfigureWidget *widget = (DrawspacesConfigureWidget *)widget_pointer;

	gedit_debug (DEBUG_PLUGINS);

	g_object_unref (widget->settings);
	g_slice_free (DrawspacesConfigureWidget, widget_pointer);

	gedit_debug_message (DEBUG_PLUGINS, "END");
}

static void
set_flag (DrawspacesConfigureWidget *widget, GtkSourceDrawSpacesFlags flag, gboolean active)
{
	widget->flags = active ? widget->flags | flag : widget->flags & ~flag;
	g_settings_set_flags (widget->settings,
			      SETTINGS_KEY_DRAW_SPACES,
			      widget->flags);
}

static void
on_draw_tabs_toggled (GtkToggleButton           *button,
		      DrawspacesConfigureWidget *widget)
{
	set_flag (widget, GTK_SOURCE_DRAW_SPACES_TAB, gtk_toggle_button_get_active (button));
}

static void
on_draw_spaces_toggled (GtkToggleButton           *button,
			DrawspacesConfigureWidget *widget)
{
	set_flag (widget, GTK_SOURCE_DRAW_SPACES_SPACE, gtk_toggle_button_get_active (button));
}

static void
on_draw_newline_toggled (GtkToggleButton           *button,
			 DrawspacesConfigureWidget *widget)
{
	set_flag (widget, GTK_SOURCE_DRAW_SPACES_NEWLINE, gtk_toggle_button_get_active (button));
}

static void
on_draw_nbsp_toggled (GtkToggleButton           *button,
		      DrawspacesConfigureWidget *widget)
{
	set_flag (widget, GTK_SOURCE_DRAW_SPACES_NBSP, gtk_toggle_button_get_active (button));
}

static void
on_draw_leading_toggled (GtkToggleButton           *button,
			 DrawspacesConfigureWidget *widget)
{
	set_flag (widget, GTK_SOURCE_DRAW_SPACES_LEADING, gtk_toggle_button_get_active (button));
}

static void
on_draw_text_toggled (GtkToggleButton           *button,
		      DrawspacesConfigureWidget *widget)
{
	set_flag (widget, GTK_SOURCE_DRAW_SPACES_TEXT, gtk_toggle_button_get_active (button));
}

static void
on_draw_trailing_toggled (GtkToggleButton           *button,
			  DrawspacesConfigureWidget *widget)
{
	set_flag (widget, GTK_SOURCE_DRAW_SPACES_TRAILING, gtk_toggle_button_get_active (button));
}

static DrawspacesConfigureWidget *
get_configuration_widget (GeditDrawspacesWindowActivatable *window_activatable)
{
	DrawspacesConfigureWidget *widget = NULL;
	GtkBuilder *builder;

	gchar *root_objects[] = {
		"content",
		NULL
	};

	widget = g_slice_new (DrawspacesConfigureWidget);
	widget->settings = g_settings_new (DRAWSPACES_SETTINGS_BASE);
	widget->flags = g_settings_get_flags (widget->settings,
					      SETTINGS_KEY_DRAW_SPACES);

	builder = gtk_builder_new ();
	gtk_builder_set_translation_domain (builder, GETTEXT_PACKAGE);
	gtk_builder_add_objects_from_resource (builder, "/org/gnome/gedit/window_activatables/drawspaces/ui/gedit-drawspaces-configurable.ui",
	                                       root_objects, NULL);
	widget->content = GTK_WIDGET (gtk_builder_get_object (builder, "content"));
	g_object_ref (widget->content);
	widget->draw_tabs = GTK_WIDGET (gtk_builder_get_object (builder, "check_button_draw_tabs"));
	widget->draw_spaces = GTK_WIDGET (gtk_builder_get_object (builder, "check_button_draw_spaces"));
	widget->draw_newline = GTK_WIDGET (gtk_builder_get_object (builder, "check_button_draw_new_lines"));
	widget->draw_nbsp = GTK_WIDGET (gtk_builder_get_object (builder, "check_button_draw_nbsp"));
	widget->draw_leading = GTK_WIDGET (gtk_builder_get_object (builder, "check_button_draw_leading"));
	widget->draw_text = GTK_WIDGET (gtk_builder_get_object (builder, "check_button_draw_text"));
	widget->draw_trailing = GTK_WIDGET (gtk_builder_get_object (builder, "check_button_draw_trailing"));
	g_object_unref (builder);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget->draw_tabs),
				      widget->flags & GTK_SOURCE_DRAW_SPACES_TAB);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget->draw_spaces),
				      widget->flags & GTK_SOURCE_DRAW_SPACES_SPACE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget->draw_newline),
				      widget->flags & GTK_SOURCE_DRAW_SPACES_NEWLINE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget->draw_nbsp),
				      widget->flags & GTK_SOURCE_DRAW_SPACES_NBSP);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget->draw_leading),
				      widget->flags & GTK_SOURCE_DRAW_SPACES_LEADING);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget->draw_text),
				      widget->flags & GTK_SOURCE_DRAW_SPACES_TEXT);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget->draw_trailing),
				      widget->flags & GTK_SOURCE_DRAW_SPACES_TRAILING);

	g_signal_connect (widget->draw_tabs,
			  "toggled",
			  G_CALLBACK (on_draw_tabs_toggled),
			  widget);
	g_signal_connect (widget->draw_spaces,
			  "toggled",
			  G_CALLBACK (on_draw_spaces_toggled),
			  widget);
	g_signal_connect (widget->draw_newline,
			  "toggled",
			  G_CALLBACK (on_draw_newline_toggled),
			  widget);
	g_signal_connect (widget->draw_nbsp,
			  "toggled",
			  G_CALLBACK (on_draw_nbsp_toggled),
			  widget);
	g_signal_connect (widget->draw_leading,
			  "toggled",
			  G_CALLBACK (on_draw_leading_toggled),
			  widget);
	g_signal_connect (widget->draw_text,
			  "toggled",
			  G_CALLBACK (on_draw_text_toggled),
			  widget);
	g_signal_connect (widget->draw_trailing,
			  "toggled",
			  G_CALLBACK (on_draw_trailing_toggled),
			  widget);

	g_signal_connect (widget->content,
			  "destroy",
			  G_CALLBACK (widget_destroyed),
			  widget);

	return widget;
}

static GtkWidget *
gedit_drawspaces_window_activatable_create_configure_widget (PeasGtkConfigurable *configurable)
{
	DrawspacesConfigureWidget *widget;

	widget = get_configuration_widget (GEDIT_DRAWSPACES_WINDOW_ACTIVATABLE (configurable));

	return widget->content;
}

static void
gedit_drawspaces_window_activatable_class_init (GeditDrawspacesWindowActivatableClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = gedit_drawspaces_window_activatable_dispose;
	object_class->set_property = gedit_drawspaces_window_activatable_set_property;
	object_class->get_property = gedit_drawspaces_window_activatable_get_property;

	g_object_class_override_property (object_class, PROP_WINDOW, "window");

	g_type_class_add_private (object_class, sizeof (GeditDrawspacesWindowActivatablePrivate));
}

static void
gedit_drawspaces_window_activatable_class_finalize (GeditDrawspacesWindowActivatableClass *klass)
{
}

static void
peas_gtk_configurable_iface_init (PeasGtkConfigurableInterface *iface)
{
	iface->create_configure_widget = gedit_drawspaces_window_activatable_create_configure_widget;
}

static void
gedit_window_activatable_iface_init (GeditWindowActivatableInterface *iface)
{
	iface->activate = gedit_drawspaces_window_activatable_window_activate;
	iface->deactivate = gedit_drawspaces_window_activatable_window_deactivate;
}

void
gedit_drawspaces_window_activatable_register (GTypeModule *module)
{
	gedit_drawspaces_window_activatable_register_type (module);

	peas_object_module_register_extension_type (PEAS_OBJECT_MODULE (module),
						    GEDIT_TYPE_WINDOW_ACTIVATABLE,
						    GEDIT_TYPE_DRAWSPACES_WINDOW_ACTIVATABLE);
	peas_object_module_register_extension_type (PEAS_OBJECT_MODULE (module),
						    PEAS_GTK_TYPE_CONFIGURABLE,
						    GEDIT_TYPE_DRAWSPACES_WINDOW_ACTIVATABLE);
}

/* ex:set ts=8 noet: */
