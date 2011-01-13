/*
 * Maemo TV out control home plugin
 * Copyright (C) 2010  Ville Syrjälä <syrjala@sci.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef TVOUT_HOME_PLUGIN_H
#define TVOUT_HOME_PLUGIN_H

#include <glib-object.h>
#include <libhildondesktop/libhildondesktop.h>

G_BEGIN_DECLS

#define TVOUT_TYPE_HOME_PLUGIN \
  (tvout_home_plugin_get_type ())
#define TVOUT_HOME_PLUGIN(obj) \
  G_TYPE_CHECK_INSTANCE_CAST ((obj), TVOUT_TYPE_HOME_PLUGIN, TVoutHomePlugin)
#define TVOUT_HOME_PLUGIN_CLASS(klass) \
  G_TYPE_CHECK_CLASS_CAST ((klass), TVOUT_TYPE_HOME_PLUGIN, TVoutHomePluginClass)
#define TVOUT_IS_HOME_PLUGIN(obj) \
  G_TYPE_CHECK_INSTANCE_TYPE ((obj), TVOUT_TYPE_HOME_PLUGIN)
#define TVOUT_IS_HOME_PLUGIN_CLASS(klass) \
  G_TYPE_CHECK_CLASS_TYPE ((klas), TVOUT_TYPE_HOME_PLUGIN)
#define TVOUT_HOME_PLUGIN_GET_CLASS(klass) \
  G_TYPE_INSTANCE_GET_CLASS ((obj), TVOUT_TYPE_HOME_PLUGIN, TVoutHomePluginClass)

typedef struct _TVoutHomePlugin TVoutHomePlugin;
typedef struct _TVoutHomePluginClass TVoutHomePluginClass;
typedef struct _TVoutHomePluginPrivate TVoutHomePluginPrivate;

struct _TVoutHomePlugin
{
  HDHomePluginItem parent;

  TVoutHomePluginPrivate *priv;
};

struct _TVoutHomePluginClass
{
  HDHomePluginItemClass parent;
};

GType tvout_home_plugin_get_type (void);

G_END_DECLS

#endif
