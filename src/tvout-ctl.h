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

typedef struct _TVoutCtl TVoutCtl;

TVoutCtl *tvout_ctl_init (void *ui_data);
void tvout_ctl_exit (TVoutCtl *ctl);

int tvout_ctl_fd (TVoutCtl *ctl);
void tvout_ctl_fd_ready (TVoutCtl *ctl);

void tvout_ctl_set_enable (TVoutCtl *ctl, int value);
void tvout_ctl_set_tv_std (TVoutCtl *ctl, int value);
void tvout_ctl_set_aspect (TVoutCtl *ctl, int value);
void tvout_ctl_set_scale (TVoutCtl *ctl, int value);

int tvout_ctl_get_enable (TVoutCtl *ctl);
int tvout_ctl_get_tv_std (TVoutCtl *ctl);
int tvout_ctl_get_aspect (TVoutCtl *ctl);
int tvout_ctl_get_scale (TVoutCtl *ctl);

void tvout_ui_set_enable (void *ui_data, int value);
void tvout_ui_set_tv_std (void *ui_data, int value);
void tvout_ui_set_aspect (void *ui_data, int value);
void tvout_ui_set_scale (void *ui_data, int value);

#endif
