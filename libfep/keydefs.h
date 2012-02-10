/*
 * Copyright (C) 2012 Daiki Ueno <ueno@unixuser.org>
 * Copyright (C) 2012 Red Hat, Inc.
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
 */

#ifndef __LIBFEP_KEYDEFS_H__
#define __LIBFEP_KEYDEFS_H__

/**
 * SECTION:keydefs
 * @short_description: Constant definitions for keyval/modifiers
 */

/**
 * FepModifierType:
 * @FEP_SHIFT_MASK: the Shift key.
 * @FEP_LOCK_MASK: a Lock key (depending on the modifier mapping of the
 *  X server this may either be CapsLock or ShiftLock).
 * @FEP_CONTROL_MASK: the Control key.
 * @FEP_MOD1_MASK: the fourth modifier key (it depends on the modifier
 *  mapping of the X server which key is interpreted as this modifier, but
 *  normally it is the Alt key).
 * @FEP_MOD2_MASK: the fifth modifier key (it depends on the modifier
 *  mapping of the X server which key is interpreted as this modifier).
 * @FEP_MOD3_MASK: the sixth modifier key (it depends on the modifier
 *  mapping of the X server which key is interpreted as this modifier).
 * @FEP_MOD4_MASK: the seventh modifier key (it depends on the modifier
 *  mapping of the X server which key is interpreted as this modifier).
 * @FEP_MOD5_MASK: the eighth modifier key (it depends on the modifier
 *  mapping of the X server which key is interpreted as this modifier).
 * @FEP_SUPER_MASK: the Super modifier. Since 2.10
 * @FEP_HYPER_MASK: the Hyper modifier. Since 2.10
 * @FEP_META_MASK: the Meta modifier. Since 2.10
 * @FEP_RELEASE_MASK: not used in FEP itself. GTK+ uses it to differentiate
 *  between (keyval, modifiers) pairs from key press and release events.
 */
typedef enum  {
  FEP_SHIFT_MASK = 1 << 0,
  FEP_LOCK_MASK = 1 << 1,
  FEP_CONTROL_MASK = 1 << 2,
  FEP_MOD1_MASK = 1 << 3,
  FEP_MOD2_MASK = 1 << 4,
  FEP_MOD3_MASK = 1 << 5,
  FEP_MOD4_MASK = 1 << 6,
  FEP_MOD5_MASK = 1 << 7,
  FEP_SUPER_MASK = 1 << 26,
  FEP_HYPER_MASK = 1 << 27,
  FEP_META_MASK = 1 << 28,
  FEP_RELEASE_MASK = 1 << 30
} FepModifierType;

#define FEP_Left 0xff51
#define FEP_Up 0xff52
#define FEP_Right 0xff53
#define FEP_Down 0xff54
#define FEP_BackSpace 0xff08
#define FEP_Delete 0xffff
#define FEP_Prior 0xff55
#define FEP_Next 0xff56
#define FEP_Home 0xff50
#define FEP_End 0xff57
#define FEP_Insert 0xff63
#define FEP_F1 0xffbe
#define FEP_F2 0xffbf
#define FEP_F3 0xffc0
#define FEP_F4 0xffc1
#define FEP_F5 0xffc2
#define FEP_F6 0xffc3
#define FEP_F7 0xffc4
#define FEP_F8 0xffc5
#define FEP_F9 0xffc6
#define FEP_F10 0xffc7
#define FEP_F11 0xffc8
#define FEP_F12 0xffc9
#define FEP_Tab 0xff09
#define FEP_Return 0xff0d
#define FEP_Escape 0xff1b

#endif	/* __LIBFEP_KEYDEFS_H__ */
