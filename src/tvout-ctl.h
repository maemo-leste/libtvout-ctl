/*
 * Maemo TV out control
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

#ifndef TVOUT_CTL_H
#define TVOUT_CTL_H

#include <glib.h>

typedef struct _TVoutCtl TVoutCtl;

TVoutCtl *tvout_ctl_init (gpointer ui_data);
void tvout_ctl_exit (TVoutCtl *ctl);

void tvout_ctl_set_enable (TVoutCtl *ctl, gint value);
void tvout_ctl_set_tv_std (TVoutCtl *ctl, gint value);
void tvout_ctl_set_aspect (TVoutCtl *ctl, gint value);
void tvout_ctl_set_scale (TVoutCtl *ctl, gint value);

gint tvout_ctl_get_enable (TVoutCtl *ctl);
gint tvout_ctl_get_tv_std (TVoutCtl *ctl);
gint tvout_ctl_get_aspect (TVoutCtl *ctl);
gint tvout_ctl_get_scale (TVoutCtl *ctl);

void tvout_ui_set_enable (gpointer ui_data, gint value);
void tvout_ui_set_tv_std (gpointer ui_data, gint value);
void tvout_ui_set_aspect (gpointer ui_data, gint value);
void tvout_ui_set_scale (gpointer ui_data, gint value);

#endif
