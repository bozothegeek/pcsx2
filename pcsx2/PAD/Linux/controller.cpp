/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2020  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PAD.h"
#include "controller.h"

__forceinline bool IsAnalogKey(int index)
{
	return ((index >= PAD_L_UP) && (index <= PAD_R_LEFT));
}

/// g_key_status.press but with proper handling for analog buttons
__forceinline void PressButton(u32 pad, u32 button)
{
	// Analog controls.
	if (IsAnalogKey(button))
	{
		switch (button)
		{
			case PAD_R_LEFT:
			case PAD_R_UP:
			case PAD_L_LEFT:
			case PAD_L_UP:
				g_key_status.press(pad, button, -MAX_ANALOG_VALUE);
				break;
			case PAD_R_RIGHT:
			case PAD_R_DOWN:
			case PAD_L_RIGHT:
			case PAD_L_DOWN:
				g_key_status.press(pad, button, MAX_ANALOG_VALUE);
				break;
		}
	}
	else
	{
		g_key_status.press(pad, button);
	}
}