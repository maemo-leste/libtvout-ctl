/*
 * Maemo TV out control
 * Copyright (C) 2010-2012  Ville Syrjälä <syrjala@sci.fi>
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

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _TVoutCtl TVoutCtl;

enum TVoutCtlAttr {
	TVOUT_CTL_ENABLE,
	TVOUT_CTL_TV_STD,
	TVOUT_CTL_ASPECT,
	TVOUT_CTL_SCALE,
	TVOUT_CTL_DYNAMIC_ASPECT,
	TVOUT_CTL_XOFFSET,
	TVOUT_CTL_YOFFSET,
	TVOUT_CTL_FULLSCREEN_VIDEO,
};

typedef void (*TVoutCtlNotify)(void *ui_data, enum TVoutCtlAttr attr, int value);

TVoutCtl *tvout_ctl_init(TVoutCtlNotify ui_notify, void *ui_data);

void tvout_ctl_exit(TVoutCtl *ctl);

int tvout_ctl_fd(TVoutCtl *ctl);
void tvout_ctl_fd_ready(TVoutCtl *ctl);

int tvout_ctl_set(TVoutCtl *ctl, enum TVoutCtlAttr attr, int value);
int tvout_ctl_get(TVoutCtl *ctl, enum TVoutCtlAttr attr);

#ifdef __cplusplus
}
#endif

#endif
