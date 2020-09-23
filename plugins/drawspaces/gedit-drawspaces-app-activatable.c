/*
 * Copyright (C) 2008-2014 Ignacio Casal Quinteiro <icq@gnome.org>
 * Copyright (C) 2020 Sébastien Wilmet <swilmet@gnome.org>
 *
 * gedit is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * gedit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gedit. If not, see <http://www.gnu.org/licenses/>.
 */

#include "gedit-drawspaces-app-activatable.h"
#include "gedit-drawspaces-view-activatable.h"
#include <tepl/tepl.h>
#include <libpeas-gtk/peas-gtk-configurable.h>

/* TODO: rename the class to GeditDrawspacesPrefs or something like that, since
 * it no longer implements the GeditAppActivatable interface.
 */

typedef struct _GeditDrawspacesAppActivatablePrivate
{
	GSettings *settings;
} GeditDrawspacesAppActivatablePrivate;

static void peas_gtk_configurable_iface_init (PeasGtkConfigurableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (GeditDrawspacesAppActivatable,
				gedit_drawspaces_app_activatable,
				G_TYPE_OBJECT,
				0,
				G_ADD_PRIVATE_DYNAMIC (GeditDrawspacesAppActivatable)
				G_IMPLEMENT_INTERFACE_DYNAMIC (PEAS_GTK_TYPE_CONFIGURABLE,
							       peas_gtk_configurable_iface_init))

static void
gedit_drawspaces_app_activatable_dispose (GObject *object)
{
	GeditDrawspacesAppActivatable *activatable = GEDIT_DRAWSPACES_APP_ACTIVATABLE (object);
	GeditDrawspacesAppActivatablePrivate *priv = gedit_drawspaces_app_activatable_get_instance_private (activatable);

	g_clear_object (&priv->settings);

	G_OBJECT_CLASS (gedit_drawspaces_app_activatable_parent_class)->dispose (object);
}

static void
gedit_drawspaces_app_activatable_class_init (GeditDrawspacesAppActivatableClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = gedit_drawspaces_app_activatable_dispose;
}

static void
gedit_drawspaces_app_activatable_class_finalize (GeditDrawspacesAppActivatableClass *klass)
{
}

static void
gedit_drawspaces_app_activatable_init (GeditDrawspacesAppActivatable *activatable)
{
	GeditDrawspacesAppActivatablePrivate *priv = gedit_drawspaces_app_activatable_get_instance_private (activatable);

	priv->settings = g_settings_new ("org.gnome.gedit.plugins.drawspaces");
}

static GtkWidget *
gedit_drawspaces_app_activatable_create_configure_widget (PeasGtkConfigurable *configurable)
{
	GeditDrawspacesAppActivatable *activatable = GEDIT_DRAWSPACES_APP_ACTIVATABLE (configurable);
	GeditDrawspacesAppActivatablePrivate *priv = gedit_drawspaces_app_activatable_get_instance_private (activatable);
	TeplSpaceDrawerPrefs *prefs;
	GtkSourceSpaceDrawer *space_drawer;

	prefs = tepl_space_drawer_prefs_new ();
	space_drawer = tepl_space_drawer_prefs_get_space_drawer (prefs);
	gtk_source_space_drawer_bind_matrix_setting (space_drawer,
						     priv->settings, "matrix",
						     G_SETTINGS_BIND_DEFAULT |
						     G_SETTINGS_BIND_NO_SENSITIVITY);

	return GTK_WIDGET (prefs);
}

static void
peas_gtk_configurable_iface_init (PeasGtkConfigurableInterface *iface)
{
	iface->create_configure_widget = gedit_drawspaces_app_activatable_create_configure_widget;
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
	gedit_drawspaces_app_activatable_register_type (G_TYPE_MODULE (module));
	gedit_drawspaces_view_activatable_register (G_TYPE_MODULE (module));

	peas_object_module_register_extension_type (module,
	                                            PEAS_GTK_TYPE_CONFIGURABLE,
	                                            GEDIT_TYPE_DRAWSPACES_APP_ACTIVATABLE);
}

/* ex:set ts=8 noet: */
