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

#ifndef GEDIT_DRAWSPACES_APP_ACTIVATABLE_H
#define GEDIT_DRAWSPACES_APP_ACTIVATABLE_H

#include <glib-object.h>
#include <libpeas/peas-object-module.h>

G_BEGIN_DECLS

#define GEDIT_TYPE_DRAWSPACES_APP_ACTIVATABLE			(gedit_drawspaces_app_activatable_get_type ())
#define GEDIT_DRAWSPACES_APP_ACTIVATABLE(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GEDIT_TYPE_DRAWSPACES_APP_ACTIVATABLE, GeditDrawspacesAppActivatable))
#define GEDIT_DRAWSPACES_APP_ACTIVATABLE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GEDIT_TYPE_DRAWSPACES_APP_ACTIVATABLE, GeditDrawspacesAppActivatableClass))
#define GEDIT_IS_DRAWSPACES_APP_ACTIVATABLE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GEDIT_TYPE_DRAWSPACES_APP_ACTIVATABLE))
#define GEDIT_IS_DRAWSPACES_APP_ACTIVATABLE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GEDIT_TYPE_DRAWSPACES_APP_ACTIVATABLE))
#define GEDIT_DRAWSPACES_APP_ACTIVATABLE_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), GEDIT_TYPE_DRAWSPACES_APP_ACTIVATABLE, GeditDrawspacesAppActivatableClass))

typedef struct _GeditDrawspacesAppActivatable		GeditDrawspacesAppActivatable;
typedef struct _GeditDrawspacesAppActivatableClass	GeditDrawspacesAppActivatableClass;

struct _GeditDrawspacesAppActivatable
{
	GObject parent;
};

struct _GeditDrawspacesAppActivatableClass
{
	GObjectClass parent_class;
};

GType                   gedit_drawspaces_app_activatable_get_type   (void);

G_MODULE_EXPORT void    peas_register_types                         (PeasObjectModule *module);

G_END_DECLS

#endif /* GEDIT_DRAWSPACES_APP_ACTIVATABLE_H */
