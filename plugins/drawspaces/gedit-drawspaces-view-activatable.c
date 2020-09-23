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

#include "gedit-drawspaces-view-activatable.h"
#include <gedit/gedit-view.h>
#include <gedit/gedit-view-activatable.h>
#include <libpeas/peas-object-module.h>

typedef struct _GeditDrawspacesViewActivatablePrivate
{
	GeditView *view;
	GSettings *settings;
} GeditDrawspacesViewActivatablePrivate;

enum
{
	PROP_0,
	PROP_VIEW
};

static void gedit_view_activatable_iface_init (GeditViewActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (GeditDrawspacesViewActivatable,
				gedit_drawspaces_view_activatable,
				G_TYPE_OBJECT,
				0,
				G_ADD_PRIVATE_DYNAMIC (GeditDrawspacesViewActivatable)
				G_IMPLEMENT_INTERFACE_DYNAMIC (GEDIT_TYPE_VIEW_ACTIVATABLE,
							       gedit_view_activatable_iface_init))

static void
gedit_drawspaces_view_activatable_dispose (GObject *object)
{
	GeditDrawspacesViewActivatable *activatable = GEDIT_DRAWSPACES_VIEW_ACTIVATABLE (object);
	GeditDrawspacesViewActivatablePrivate *priv = gedit_drawspaces_view_activatable_get_instance_private (activatable);

	g_clear_object (&priv->view);
	g_clear_object (&priv->settings);

	G_OBJECT_CLASS (gedit_drawspaces_view_activatable_parent_class)->dispose (object);
}

static void
gedit_drawspaces_view_activatable_set_property (GObject      *object,
                                                guint         prop_id,
                                                const GValue *value,
                                                GParamSpec   *pspec)
{
	GeditDrawspacesViewActivatable *activatable = GEDIT_DRAWSPACES_VIEW_ACTIVATABLE (object);
	GeditDrawspacesViewActivatablePrivate *priv = gedit_drawspaces_view_activatable_get_instance_private (activatable);

	switch (prop_id)
	{
		case PROP_VIEW:
			priv->view = GEDIT_VIEW (g_value_dup_object (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gedit_drawspaces_view_activatable_get_property (GObject    *object,
                                                guint       prop_id,
                                                GValue     *value,
                                                GParamSpec *pspec)
{
	GeditDrawspacesViewActivatable *activatable = GEDIT_DRAWSPACES_VIEW_ACTIVATABLE (object);
	GeditDrawspacesViewActivatablePrivate *priv = gedit_drawspaces_view_activatable_get_instance_private (activatable);

	switch (prop_id)
	{
		case PROP_VIEW:
			g_value_set_object (value, priv->view);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gedit_drawspaces_view_activatable_class_init (GeditDrawspacesViewActivatableClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = gedit_drawspaces_view_activatable_dispose;
	object_class->set_property = gedit_drawspaces_view_activatable_set_property;
	object_class->get_property = gedit_drawspaces_view_activatable_get_property;

	g_object_class_override_property (object_class, PROP_VIEW, "view");
}

static void
gedit_drawspaces_view_activatable_class_finalize (GeditDrawspacesViewActivatableClass *klass)
{
}

static void
gedit_drawspaces_view_activatable_init (GeditDrawspacesViewActivatable *activatable)
{
	GeditDrawspacesViewActivatablePrivate *priv = gedit_drawspaces_view_activatable_get_instance_private (activatable);

	priv->settings = g_settings_new ("org.gnome.gedit.plugins.drawspaces");
}

static void
gedit_drawspaces_view_activatable_activate (GeditViewActivatable *activatable)
{
	GeditDrawspacesViewActivatablePrivate *priv;
	GtkSourceSpaceDrawer *space_drawer;

	priv = gedit_drawspaces_view_activatable_get_instance_private (GEDIT_DRAWSPACES_VIEW_ACTIVATABLE (activatable));

	space_drawer = gtk_source_view_get_space_drawer (GTK_SOURCE_VIEW (priv->view));
	gtk_source_space_drawer_bind_matrix_setting (space_drawer,
						     priv->settings, "matrix",
						     G_SETTINGS_BIND_GET |
						     G_SETTINGS_BIND_NO_SENSITIVITY);
	gtk_source_space_drawer_set_enable_matrix (space_drawer, TRUE);
}

static void
gedit_drawspaces_view_activatable_deactivate (GeditViewActivatable *activatable)
{
	GeditDrawspacesViewActivatablePrivate *priv;
	GtkSourceSpaceDrawer *space_drawer;

	priv = gedit_drawspaces_view_activatable_get_instance_private (GEDIT_DRAWSPACES_VIEW_ACTIVATABLE (activatable));

	space_drawer = gtk_source_view_get_space_drawer (GTK_SOURCE_VIEW (priv->view));
	g_settings_unbind (space_drawer, "matrix");
	gtk_source_space_drawer_set_enable_matrix (space_drawer, FALSE);
}

static void
gedit_view_activatable_iface_init (GeditViewActivatableInterface *iface)
{
	iface->activate = gedit_drawspaces_view_activatable_activate;
	iface->deactivate = gedit_drawspaces_view_activatable_deactivate;
}

void
gedit_drawspaces_view_activatable_register (GTypeModule *module)
{
	gedit_drawspaces_view_activatable_register_type (module);

	peas_object_module_register_extension_type (PEAS_OBJECT_MODULE (module),
						    GEDIT_TYPE_VIEW_ACTIVATABLE,
						    GEDIT_TYPE_DRAWSPACES_VIEW_ACTIVATABLE);
}

/* ex:set ts=8 noet: */
